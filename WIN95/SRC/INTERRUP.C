/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    interrup.c

Abstract:

    This is a part of the driver for the National Semiconductor Novell 2000
    Ethernet controller.  It contains the interrupt-handling routines.
    This driver conforms to the NDIS 3.0 interface.

    The overall structure and much of the code is taken from
    the Lance NDIS driver by Tony Ercolano.

Author:

    Sean Selitrennikoff (seanse) Dec-1991

Environment:

    Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:

--*/

#include <ndis.h>
#include "mpcccard.h"
#include "ne2000hw.h"
#include "ne2000sw.h"
#include "mpccfunc.h"
#include "debug.h"
#include "channel.h"
#include "const.h"
#include "extervar.h"

#define  __FILEID__     1 


// On debug builds tell the compiler to keep the symbols for
// internal functions, otw throw them out.
//
#if DBG
#define STATIC
#else
#define STATIC static
#endif



INDICATE_STATUS
Ne2000IndicatePacket(
    IN PNE2000_ADAPTER Adapter
    );

VOID
NE2000DoNextSend(
    PNE2000_ADAPTER Adapter
    );

VOID NE2000TransmitComplete(IN PNE2000_ADAPTER Adapter);
VOID TransmitQueue(IN PNE2000_ADAPTER   Adapter);

//added by fang



//
// This is used to pad short packets.
//
static UCHAR BlankBuffer[60] = "                                                            ";



VOID
Ne2000EnableInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    )

/*++

Routine Description:

    This routine is used to turn on the interrupt mask.

Arguments:

    Context - The adapter for the NE2000 to start.

Return Value:

    None.

--*/

{
    PNE2000_ADAPTER Adapter = (PNE2000_ADAPTER)(MiniportAdapterContext);

    IF_LOG( Ne2000Log('P'); )

    CardUnblockInterrupts(Adapter);

}

VOID
Ne2000DisableInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    )

/*++

Routine Description:

    This routine is used to turn off the interrupt mask.

Arguments:

    Context - The adapter for the NE2000 to start.

Return Value:

    None.

--*/

{
    PNE2000_ADAPTER Adapter = (PNE2000_ADAPTER)(MiniportAdapterContext);

    IF_LOG( Ne2000Log('p'); )

    CardBlockInterrupts(Adapter);

}

VOID
Ne2000Isr(
    OUT PBOOLEAN InterruptRecognized,
    OUT PBOOLEAN QueueDpc,
    IN PVOID Context
    )

/*++

Routine Description:

    This is the interrupt handler which is registered with the operating
    system. If several are pending (i.e. transmit complete and receive),
    handle them all.  Block new interrupts until all pending interrupts
    are handled.

Arguments:

    InterruptRecognized - Boolean value which returns TRUE if the
        ISR recognizes the interrupt as coming from this adapter.

    QueueDpc - TRUE if a DPC should be queued.

    Context - pointer to the adapter object

Return Value:

    None.
--*/

{
    PNE2000_ADAPTER Adapter = ((PNE2000_ADAPTER)Context);

    IF_LOUD( DbgPrint("In Ne2000ISR\n");)

    IF_LOG( Ne2000Log('i'); )

    IF_VERY_LOUD( DbgPrint( "Ne2000InterruptHandler entered\n" );)

    //
    // Force the INT signal from the chip low. When all
    // interrupts are acknowledged interrupts will be unblocked,
    //
    CardBlockInterrupts(Adapter);

    IF_LOG( Ne2000Log('I'); )

    *InterruptRecognized = TRUE;
    *QueueDpc = TRUE;

    return;
}

//BY FANG
VOID
Ne2000HandleInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    )
/*++

Routine Description:

    This is the defered processing routine for interrupts.  It
    reads from the Interrupt Status Register any outstanding
    interrupts and handles them.

Arguments:

    MiniportAdapterContext - a handle to the adapter block.

Return Value:

    NONE.

--*/
{

    UCHAR  uch;

    // The adapter to process
    PNE2000_ADAPTER Adapter = ((PNE2000_ADAPTER)MiniportAdapterContext);
    DBG_FUNC("HtDsuHandleInterrupt")

    // A pointer to our link information structure.
    register    retval;

    DBG_ENTER(Adapter);
	retval=i2ssetwinsave(Adapter);
	if(retval!=0){
		DbgPrint("sdlcintr cardn %d reenter\n",Adapter->InstanceNumber);
    	return;
	}
	
    NdisReadRegisterUchar( &(Adapter->pc2card_scbp->intrack),
                           &uch
                          );
                          
	if(uch!=0)
	{
		i2sresetwin(Adapter);
		DBG_NOTICE(Adapter,("ISR intrack==%d\n",
		Adapter->pc2card_scbp->intrack));
		return;
	}

    NdisReadRegisterUchar( &(Adapter->pc2card_scbp->intrflag),
                           &uch
                          );
	switch( uch ){
	case    TX0_FINISH:
			DBG_NOTICE(Adapter,("TX0_FINISH\n"));
			TransmitQueue(Adapter);
			Adapter->pc2card_scbp->intrack=1;
			i2sresetwin(Adapter);
//			DbgPrint("TX0_Finish\n");
		    DBG_LEAVE(Adapter);

	//NdisReleaseSpinLock(&Adapter->Lock);
		    return;
	case    TX1_FINISH:
			DBG_NOTICE(Adapter,("TX1_FINISH\n"));
//		    HtDsuTransmitComplete(Adapter);
			break;
	case    RXDATA0_RDY:
			Adapter->inread0=1;
			DBG_NOTICE(Adapter,("RXDATA0_RDY\n"));
			//  Need processing 
			HtDsuReceivePacket(Adapter,0);
			break;
	case    RXDATA1_RDY:
			Adapter->inread1=1;
            DBG_NOTICE(Adapter,("RXDATA1_RDY\n"));
			HtDsuReceivePacket(Adapter,1);
			break;
	case    LPRINT:
			{
			char    *str;
			char    strbuf[80];
			int	i;
		       
			str=(PUCHAR)Adapter->VirtualAddress+
			Adapter->pc2card_scbp->pstr[1]*256+
			Adapter->pc2card_scbp->pstr[0];
			for(i=0;(str[i]!=0)&&(i<79);i++){
				strbuf[i]=str[i];
			}
			strbuf[i]=0;
			DBG_NOTICE(Adapter,("LPRINT:%s\n",strbuf));
			}
			break;
	case    LEXP1_RDY:
			DBG_NOTICE(Adapter,("LEXP1_RDY\n"));
			break;
	case    LEXP0_RDY:
			DBG_NOTICE(Adapter,("LEXP0_RDY\n"));
			break;
	case    LOCAL1_RDY:
			DBG_NOTICE(Adapter,("LOCAL1_RDY\n"));
			break;
	case    LOCAL0_RDY:
			DBG_NOTICE(Adapter,("LOCAL0_RDY\n"));
			break;
	case    LINK_ST0CHANGE:
			DBG_NOTICE(Adapter,("LINK_ST0CHANGE:%d\n",Adapter->pc2card_scbp->link_state[0]));
		    if(Adapter->pc2card_scbp->link_state[0]==LINK_ACTIVE){
			 Adapter->pc2card_scbp->link_state[0]=LINK_NORMAL;
			 // Need Processing 
		 	 DBG_NOTICE(Adapter,("set state 0 CHANGE:%d\n",Adapter->pc2card_scbp->link_state[0]));
			 LapbReady(Adapter);
		    }
		    else if (Adapter->pc2card_scbp->link_state[0]==LINK_NORMAL)
					 LapbReady(Adapter);
		    	
 		    break;
	case    LINK_ST1CHANGE:
		   if(Adapter->pc2card_scbp->link_state[1]==LINK_ACTIVE){
			 Adapter->pc2card_scbp->link_state[1]=LINK_NORMAL;
		   }
			DBG_NOTICE(Adapter,("LINK_ST1CHAGE\n"));
		   break;
	default:
			DBG_ERROR(Adapter,("ISR unknow type:%d\n",Adapter->pc2card_scbp->intrflag));
	}

    NdisWriteRegisterUchar(&(Adapter->pc2card_scbp->intrack),
                           1
                           );
	//WRITE_REGISTER_UCHAR(&(Adapter->pc2card_scbp->intrack),1);
	//replaced by fang
	i2sresetwin(Adapter);

    DBG_LEAVE(Adapter);


    return;

}


INDICATE_STATUS
Ne2000IndicatePacket(
    IN PNE2000_ADAPTER Adapter
    )

/*++

Routine Description:

    Indicates the first packet on the card to the protocols.

    NOTE: For MP, non-x86 architectures, this assumes that the packet has been
    read from the card and into Adapter->PacketHeader and Adapter->Lookahead.

    NOTE: For UP x86 systems this assumes that the packet header has been
    read into Adapter->PacketHeader and the minimal lookahead stored in
    Adapter->Lookahead

Arguments:

    Adapter - pointer to the adapter block.

Return Value:

    CARD_BAD if the card should be reset;
    INDICATE_OK otherwise.

--*/

{
    //
    // Length of the packet
    //
    UINT PacketLen;

    //
    // Length of the lookahead buffer
    //
    UINT IndicateLen;

    //
    // Variables for checking if the packet header looks valid
    //
    UCHAR PossibleNextPacket1, PossibleNextPacket2;

    //
    // Check if the next packet byte agress with the length, as
    // described on p. A-3 of the Etherlink II Technical Reference.
    // The start of the packet plus the MSB of the length must
    // be equal to the start of the next packet minus one or two.
    // Otherwise the header is considered corrupted, and the
    // card must be reset.
    //

    PossibleNextPacket1 =
                Adapter->NicNextPacket + Adapter->PacketHeader[3] + (UCHAR)1;

    if (PossibleNextPacket1 >= Adapter->NicPageStop) {

        PossibleNextPacket1 -= (Adapter->NicPageStop - Adapter->NicPageStart);

    }

    if (PossibleNextPacket1 != Adapter->PacketHeader[1]) {

        PossibleNextPacket2 = PossibleNextPacket1+(UCHAR)1;

        if (PossibleNextPacket2 == Adapter->NicPageStop) {

            PossibleNextPacket2 = Adapter->NicPageStart;

        }

        if (PossibleNextPacket2 != Adapter->PacketHeader[1]) {

            IF_LOUD( DbgPrint("First CARD_BAD check failed\n"); )
            return SKIPPED;
        }

    }

    //
    // Check that the Next is valid
    //
    if ((Adapter->PacketHeader[1] < Adapter->NicPageStart) ||
        (Adapter->PacketHeader[1] > Adapter->NicPageStop)) {

        IF_LOUD( DbgPrint("Second CARD_BAD check failed\n"); )
        return(SKIPPED);

    }

    //
    // Sanity check the length
    //
    PacketLen = Adapter->PacketHeader[2] + Adapter->PacketHeader[3]*256 - 4;

    if (PacketLen > 1514) {
        IF_LOUD( DbgPrint("Third CARD_BAD check failed\n"); )
        return(SKIPPED);

    }

#if DBG

    IF_NE2000DEBUG( NE2000_DEBUG_WORKAROUND1 ) {
        //
        // Now check for the high order 2 bits being set, as described
        // on page A-2 of the Etherlink II Technical Reference. If either
        // of the two high order bits is set in the receive status byte
        // in the packet header, the packet should be skipped (but
        // the adapter does not need to be reset).
        //

        if (Adapter->PacketHeader[0] & (RSR_DISABLED|RSR_DEFERRING)) {

            IF_LOUD (DbgPrint("H");)

            return SKIPPED;

        }

    }

#endif

    //
    // Lookahead amount to indicate
    //
    IndicateLen = (PacketLen > (Adapter->MaxLookAhead + NE2000_HEADER_SIZE)) ?
                           (Adapter->MaxLookAhead + NE2000_HEADER_SIZE) :
                           PacketLen;

    //
    // Indicate packet
    //

    Adapter->PacketLen = PacketLen;

    if (IndicateLen < NE2000_HEADER_SIZE) {

        //
        // Runt Packet
        //

        NdisMEthIndicateReceive(
                Adapter->MiniportAdapterHandle,
                (NDIS_HANDLE)Adapter,
                (PCHAR)(Adapter->Lookahead),
                IndicateLen,
                NULL,
                0,
                0
                );

    } else {

        NdisMEthIndicateReceive(
                Adapter->MiniportAdapterHandle,
                (NDIS_HANDLE)Adapter,
                (PCHAR)(Adapter->Lookahead),
                NE2000_HEADER_SIZE,
                (PCHAR)(Adapter->Lookahead) + NE2000_HEADER_SIZE,
                IndicateLen - NE2000_HEADER_SIZE,
                PacketLen - NE2000_HEADER_SIZE
                );

    }

    Adapter->IndicateReceiveDone = TRUE;

    return INDICATE_OK;
}


VOID
OctogmetusceratorRevisited(
    IN PNE2000_ADAPTER Adapter
    )

/*++

Routine Description:

    Recovers the card from a transmit error.

Arguments:

    Adapter - pointer to the adapter block

Return Value:

    None.

--*/

{

    }


