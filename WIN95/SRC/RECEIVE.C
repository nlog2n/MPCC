/***************************************************************************\
|* Copyright (c) 1997  Stone  Corporation                               *|
|*                                                                         *|
|* This file is part of the BSD Communications NE2000 Miniport Driver.   *|
\***************************************************************************/
#include "version.h"


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Module Name:

    receive.c
Abstract:
    This module contains the Miniport packet receive routines.
    This driver conforms to the NDIS 3.0 Miniport interface.
Author:
	Jimmy Guo (Peking University Computer Science && Technology Department)
	July -97
Environment:
    Windows NT 3.5 kernel mode Miniport driver or equivalent.
Revision History:
---------------------------------------------------------------------------*/

#define  __FILEID__     6       // Unique file ID for error logging

#include <ndis.h>
#include "mpcccard.h"
#include "ne2000hw.h"
#include "ne2000sw.h"
#include "keywords.h"
#include "mpccfunc.h"
#include "typdef.h"
#include "channel.h"
#include "extervar.h"
#include "const.h"
#include "func.h"
#include "debug.h"

VOID NE2000TransmitComplete(IN PNE2000_ADAPTER Adapter);
//added by fang


VOID NE2000IndicatePacket(
	IN PNE2000_ADAPTER    Adapter,
	IN struct LogicalChannel *LC
)
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Routine Description:
    This routine is called from HandleDataPacket to  indicate the arrival of a
    full data unit.
    We examine the adapter memory beginning where the adapter would have
    stored the next packet.
    As we find each good packet it is passed up to the protocol stack.
    This code assumes that the receive buffer holds a single packet.
    NOTE: The Adapter->Lock must be held before calling this routine.
Arguments:
    Adapter _ A pointer ot our adapter information structure.
	LC      - A pointer to the logical channel structure
Return Value:
	None.
Revision History:
	
---------------------------------------------------------------------------*/
{
	UINT         BytesReceived;
	NDIS_STATUS Status;

	// Length of the lookahead buffer
    
    UINT IndicateLen;
    UINT tmp;
    PTableItem  RouteEntry;	   // Route table entry



	DBG_FUNC("NE2000IndicatePacket")
	DBG_ENTER(Adapter);

	RouteEntry = &Adapter->RouteTable[LC->Fd];

	
	BytesReceived = AssemblePacket(Adapter,LC,&Adapter->DataBuf[NE2000_HEADER_SIZE],
					1532,T_NORMAL);
	//BytesReceived = AssemblePacket(Adapter,LC,Adapter->DataBuf,
	//				1532,T_NORMAL);

				
	// Emulate the 802.3 header for the x25 frame 
	// Begin filling 
	// Fill the destination Address and source Address
	for (tmp =0;tmp <6;tmp++) {
		// Destination ethernet address
		Adapter->DataBuf[tmp]=Adapter->PermanentAddress[tmp];
		// Source ethernet address
		Adapter->DataBuf[tmp+6]=((tmp<2)?0:RouteEntry->IpAddress[tmp-2]);
	}	
	// Fill the packet type 
	Adapter->DataBuf[12]=0x08;
	Adapter->DataBuf[13]=0x00;
	//Adapter->DataBuf[13]=(Adapter->NotFirstFrame?0x00:0x06);
	if (!Adapter->NotFirstFrame)
		Adapter->NotFirstFrame = TRUE;
		
	Adapter->PacketLen = BytesReceived + NE2000_HEADER_SIZE;
	//Adapter->PacketLen = BytesReceived;
	if (BytesReceived < 0)
	{
		DbgPrint("Error,data assemble failed!\n");
		return;
	}

// Indicate the received packet to the upper layer.
	
//	NdisReleaseSpinLock(&Adapter->Lock);

	//
    // Lookahead amount to indicate
    //
    IndicateLen = ((BytesReceived+NE2000_HEADER_SIZE) > (Adapter->MaxLookAhead + NE2000_HEADER_SIZE)) ?
                           (Adapter->MaxLookAhead + NE2000_HEADER_SIZE) :  (BytesReceived+NE2000_HEADER_SIZE);

    //IndicateLen = ((BytesReceived) > (Adapter->MaxLookAhead + NE2000_HEADER_SIZE)) ?
    //                       (Adapter->MaxLookAhead + NE2000_HEADER_SIZE) :  (BytesReceived);
    
    /*
    // Indicate receive complete if we had any that were accepted.
    */

    if (IndicateLen < NE2000_HEADER_SIZE) {
        //
        // Runt Packet
        //
        NdisMEthIndicateReceive(
                Adapter->MiniportAdapterHandle,
                (NDIS_HANDLE)Adapter,
                (PCHAR)(Adapter->DataBuf),
                IndicateLen,
                NULL,
                0,
                0
                );

    } else {
 	       NdisMEthIndicateReceive(
                Adapter->MiniportAdapterHandle,
                (NDIS_HANDLE)Adapter,
                (PCHAR)(Adapter->DataBuf),
                NE2000_HEADER_SIZE,
                &Adapter->DataBuf[NE2000_HEADER_SIZE],
                IndicateLen - NE2000_HEADER_SIZE,
                BytesReceived 
                );
           
    }
	Adapter->FramesRcvGood++;
	
    DBG_LEAVE(Adapter);
    return;

}




NDIS_STATUS
Ne2000TransferData(
    OUT PNDIS_PACKET Packet,
    OUT PUINT BytesTransferred,
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_HANDLE MiniportReceiveContext,
    IN UINT ByteOffset,
    IN UINT BytesToTransfer
    )

/*++
Routine Description:
    A protocol calls the NE2000TransferData request (indirectly via
    NdisTransferData) from within its Receive event handler
    to instruct the driver to copy the contents of the received packet
    a specified packet buffer.
Arguments:
    MiniportAdapterContext - Context registered with the wrapper, really
        a pointer to the adapter.
    MiniportReceiveContext - The context value passed by the driver on its call
    to NdisMEthIndicateReceive.  The driver can use this value to determine
    which packet, on which adapter, is being received.
    ByteOffset - An unsigned integer specifying the offset within the
    received packet at which the copy is to begin.  If the entire packet
    is to be copied, ByteOffset must be zero.

    BytesToTransfer - An unsigned integer specifying the number of bytes
    to copy.  It is legal to transfer zero bytes; this has no effect.  If
    the sum of ByteOffset and BytesToTransfer is greater than the size
    of the received packet, then the remainder of the packet (starting from
    ByteOffset) is transferred, and the trailing portion of the receive
    buffer is not modified.

    Packet - A pointer to a descriptor for the packet storage into which
    the MAC is to copy the received packet.

    BytesTransfered - A pointer to an unsigned integer.  The MAC writes
    the actual number of bytes transferred into this location.  This value
    is not valid if the return status is STATUS_PENDING.

Notes:

  - The MacReceiveContext will be a pointer to the open block for
    the packet.

--*/

{


    //
    // Variables for the number of bytes to copy, how much can be
    // copied at this moment, and the total number of bytes to copy.
    //
    UINT BytesLeft, BytesNow, BytesWanted;

    //
    // Current NDIS_BUFFER to copy into
    //
    PNDIS_BUFFER CurBuffer;

    //
    // Virtual address of the buffer.
    //
    XMIT_BUF NextBufToXmit;
    PUCHAR BufStart;
   
    //
    // Length and offset into the buffer.
    //
    UINT BufLen, BufOff;

    DBG_FUNC("NE2000TransferData")
    //
    // The adapter to transfer from.
    //
    PNE2000_ADAPTER  Adapter = ((PNE2000_ADAPTER )MiniportReceiveContext);
	DBG_ENTER(Adapter);


    //
    // Add the packet header onto the offset.
    //

    DbgPrint("Enter TransferData\n");

    ByteOffset += NE2000_HEADER_SIZE;
    //
    // See how much data there is to transfer.
    //
    if (ByteOffset+BytesToTransfer > Adapter->PacketLen) {
       if (Adapter->PacketLen < ByteOffset) {
            *BytesTransferred = 0;
            return(NDIS_STATUS_FAILURE);
        }
        BytesWanted = Adapter->PacketLen - ByteOffset;
    } else {
        BytesWanted = BytesToTransfer;
    }

    //
    // Set the number of bytes left to transfer
    //
    BytesLeft = 0;
    //
    // Get location to copy into
    //
    NdisQueryPacket(Packet, NULL, NULL, &CurBuffer, NULL);
    NdisQueryBuffer(CurBuffer, (PVOID *)&BufStart, &BufLen);
    BufOff = 0;
    
    //
    // Loop, filling each buffer in the packet until there
    // are no more buffers or the data has all been copied.

    while (CurBuffer && (BytesLeft<BytesWanted)) {
    	while ((BufOff<BufLen)&&(BytesLeft<BytesWanted))
		{
			*BufStart++=Adapter->DataBuf[ByteOffset+BytesLeft];
			BufOff++;
			BytesLeft++;
		}
		NdisGetNextBuffer(CurBuffer, &CurBuffer);
		if (CurBuffer)
		    NdisQueryBuffer(CurBuffer, (PVOID *)&BufStart, &BufLen);
  		
    }

	//Adapter->Indicating = FALSE;
	
	DBG_LEAVE(Adapter);


	
    return(NDIS_STATUS_SUCCESS);
}

VOID HandleRestartReqPacket(
	IN PNE2000_ADAPTER    Adapter,
	IN struct PktHead   head,
	IN PUCHAR           Data_buf,
	IN INT              Data_len,
	IN UCHAR            lcn,
	IN UCHAR            Mark
)
//************************************************************
// Function to Process the restart_req_packet Arrival Event
// Processing include
// 1.     Receive the   restart_req packet
// 2.     Up_load the   restart_req to the upper layer(ipx25 router)
//************************************************************
{

	u_char  ch, ps, pr, *pstr;
	u_int   len, k1, k2;
	int        Fd, ret, i;
	PKT_TYP buff;
	struct NetAddr s_addr, d_addr,t_addr;
	struct LogicalChannel *LC;


	DBG_FUNC("HandleRestartReqPacket")
	DBG_ENTER(Adapter);

//  Initialization of the local variable
	len  = Data_len;	
	ch   = Mark;
	pstr = Data_buf;
	ASSERT((0<=lcn)&&(lcn<VC_NUM));
	LC=&Adapter->Channel[lcn];
	pstr=Data_buf;

	
	if(LC->LCN!=0)
	{
		enable();
		return;
	}
	
	switch(LC->State) {
		case RST_REQ:        /* R2 */
		    RestartPacketLevel(Adapter);
			LC->State=READY;
			if (len < 4)  {
				LC->State=LC_ERROR;
				return;
			}
			LC->Cause=*pstr++;
			if (len==5)
				LC->DiagCode=*pstr++;
			if (len > 5)  {
				LC->State=LC_ERROR;
				enable();
				return;
				}
				break;
		case RST_IND:
			if (len < 4)  {
				LC->State=LC_ERROR;
				enable();
				return;
			}
			LC->Cause=*pstr++;
			if (len==5)
				LC->DiagCode=*pstr++;
				if (len > 5)  {
				LC->State=LC_ERROR;
					enable();
					return;
				}
				break;
		default:
			LC->State=RST_IND;
			if (len < 4)  {
				LC->State=LC_ERROR;
				enable();
				return;
			}
			LC->Cause=*pstr++;
			if (len==5)
				LC->DiagCode=*pstr++;
			if (len > 5)  {
				LC->State=LC_ERROR;
				enable();
				return;
			}
			ret=SendRestartPkt(Adapter,DTE_RST_CONF, 0, 0);
			if (ret < 0) {
			  LC->State=LC_ERROR;
			  break;
			 } 
			RestartPacketLevel(Adapter);
 			break;
		}

}


VOID HandleRestartConfPacket(
	IN PNE2000_ADAPTER    Adapter,
	IN struct PktHead   head,
	IN PUCHAR           Data_buf,
	IN INT              Data_len,
	IN UCHAR            lcn,
	IN UCHAR            Mark
)
//************************************************************
// Function to Process the restart_conf_packet Arrival Event
// Processing include
// 1.     Receive the   restart_conf packet
// 2.     Up_load the   restart_conf to the upper layer(ipx25 router)
//************************************************************
{
	u_char  ch, ps, pr, *pstr;
	u_int   len, k1, k2;
	int        Fd, ret, i;
	PKT_TYP buff;
	struct NetAddr s_addr, d_addr,t_addr;
	struct LogicalChannel *LC;
	BOOLEAN  result;


	DBG_FUNC("HandleRestartConfPacket")
	DBG_ENTER(Adapter);

//  Initialization of the local variable
	len  = Data_len;	
	ch   = Mark;
	pstr = Data_buf;
	ASSERT((0<=lcn)&&(lcn<VC_NUM));
	LC=&Adapter->Channel[lcn];
	pstr=Data_buf;

	// restart_req packet on invalid channel 
	if(LC->LCN!=0)	{
		enable();
		return;
	}

	if(LC->State==RST_REQ)
	{
		// Operation to stop the timer
		// DbgPrint("Cancel\n");
		//NdisMCancelTimer(&LC->ChannelTimer,&result);
		// End of the operation 
	    RestartPacketLevel(Adapter);
	}
	else 	LC->State=LC_ERROR;
}


VOID HandleResetReqPacket(
	IN PNE2000_ADAPTER    Adapter,
	IN struct PktHead   head,
	IN PUCHAR           Data_buf,
	IN INT              Data_len,
	IN struct LogicalChannel *LC,
	IN UCHAR            Mark
)
//************************************************************
// Function to Process the reset_req_packet Arrival Event
// Processing include
// 1.     Receive the   reset_req packet
// 2.     Up_load the   reset_req to the upper layer(ipx25 router)
//************************************************************
{
	u_char  ch, ps, pr, *pstr;
	int     len;
	int     M_flag;
	int     ret,i;
	BOOLEAN result;

	DBG_FUNC("HandleResetReqPacket")
 	DBG_ENTER(Adapter);
	
//	Initialization of the variable
//  Begin of section 
	ch = Mark;
	len = Data_len;
	pstr = Data_buf;
//  End of section 	

	if (len < 4)  {
		LC->DiagCode=PacketTooShort;
		X25ErrorHandler(Adapter,LC->DiagCode,LC);
		return;
	}
	if (len > 5)  {
		LC->DiagCode=PacketTooLong;
		X25ErrorHandler(Adapter,LC->DiagCode,LC);
		return;
	}
	
			

/*** process reset indication packet ***/
 	switch(LC->State)  {
		case RES_REQ:        /* D2 */
		case FLOW_CTL:       /* D1 */
			if ((LC->State==RES_REQ)||(LC->SendQue != LAST_PKT))
			{	// Stop the orginally set timer 
			//	NdisMCancelTimer(&LC->ChannelTimer,&result);
			}
			
			// Upload the reset event to the upper level 
			// if the channel is transmitting data 
			if (LC->State == FLOW_CTL)
				UpLoadResetReq(Adapter,LC);

			LC->Cause=*pstr++;
			if (len==5)
				LC->DiagCode=*pstr++;
				
			LC->State = RES_REQ;				
			ret=SendResetPkt(Adapter,head.LCN, DTE_RES_CONF, (UCHAR)NULL,(UCHAR)NULL);

			// Send reset answer packet error 
			if (ret < 0) {
				// Oh! My god , poor card! why fail ?
				return;
			}	
			// Initialize the channel 
			ChannelInitialize(Adapter,head.LCN);
			break;
		case RES_IND:
			ret=SendResetPkt(Adapter,head.LCN, DTE_RES_CONF, (UCHAR)NULL,(UCHAR)NULL);
			
			if (ret < 0) {
				return;
			}	
			// Initialize the channel 
			ChannelInitialize(Adapter,head.LCN);
			break;
		default:
			LC->State = RES_REQ;				
			ret=SendResetPkt(Adapter,head.LCN, DTE_RES_CONF, (UCHAR)NULL,(UCHAR)NULL);

			// Send reset answer packet error 
			if (ret < 0) {
				// Oh! My god , poor card! why fail ?
				return;
			}	
			// Initialize the channel 
			ChannelInitialize(Adapter,head.LCN);
			UpLoadResetReq(Adapter,LC);
			
			//LC->State=LC_ERROR;
			// reset on other state of logical channel 
			// no operation at all 
			break;
	}

	DBG_LEAVE(Adapter);
	
}


VOID HandleResetConfPacket(
	IN PNE2000_ADAPTER    Adapter,
	IN struct PktHead   head,
	IN PUCHAR           Data_buf,
	IN INT              Data_len,
	IN struct LogicalChannel *LC,
	IN UCHAR            Mark
)
//************************************************************
// Function to Process the reset_conf_packet Arrival Event
// Processing include
// 1.     Receive the   reset_conf packet
// 2.     Up_load the   reset_conf to the upper layer(ipx25 router)
//************************************************************
{
	u_char  ch, ps, pr, *pstr;
	int     len;
	int     M_flag;
	int     ret,i;
	BOOLEAN result;

	DBG_FUNC("HandleResetConfPacket")
 	DBG_ENTER(Adapter);
	
//	Initialization of the variable
//  Begin of section 
	ch = Mark;
	len = Data_len;
	pstr = Data_buf;
//  End of section 	


	/*** process reset confirmation packet ***/
	if (len != 3)  {
		LC->State=LC_ERROR;
		enable();
		return;
	}

//  Reset operation succeed 		
	if (LC->State == RES_REQ) {   /* D2 */
		//  Stop the Orginally set timer 
//		 	NdisMCancelTimer(&LC->ChannelTimer,&result);
		//  End of operation 	
			ChannelInitialize(Adapter,LC->LCN);
	}
	else {
			LC->State=LC_ERROR;
	}
	DBG_LEAVE(Adapter);

}

VOID HandleDataPacket(
	IN PNE2000_ADAPTER    Adapter,
	IN struct PktHead   head,
	IN PUCHAR           Data_buf,
	IN INT              Data_len,
	IN struct LogicalChannel *LC,
	IN UCHAR           Mark
)
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Routine Description:
   Function to Process the Data_packet Arrival Event
   Processing include
   1.     Acknowledge information processing
   2.     Data_packet receive     processing
---------------------------------------------------------------------------*/
{
	u_char  ch, ps, pr, *pstr;
	int     len;
	int     ArrivedPacket;
	int     M_flag;
	int     ret,i;
	BOOLEAN result;

	DBG_FUNC("HandleDataPacket")
 	DBG_ENTER(Adapter);
	
//	Initialization of the variable
//  Begin of section 
	ch = Mark;
	len = Data_len;
	pstr = Data_buf;
	ArrivedPacket = 0;
//  End of section 	

//  Attempt to send data at an invalid state
	if (LC->State != DATA_TRANS)  {
		DbgPrint("State is not valid!!");
		LC->State=LC_ERROR;
		return;
	}

// Get the ps and pr from the packet mark character
	switch(LC->Modul) {
		case 0x10:
			/* 0x10==0001,0000, is module 8 */
			/* 0x0e==0000,1110 */
			ps=(ch & 0x0e) >> 1;
			pr=(ch & 0xe0)>>5;
			len-=3;
			break;
		case 0x20:
			/* 0x20==0010,0000, is module 128 */
			if (len < 4)  {
				LC->State=LC_ERROR;
				DbgPrint("Packet length error<4\n");
				enable();
				return;
			}
			ps=ch >> 1;
			ch=*pstr++;
			pr=ch >> 1;
			len-=4;
			break;
	}    // End of switch 



	// Frame number is inconsistent
	// Discard the packet and set the state 
	// REJ packet is omited.
	

    EnterX25CriticalRegion(Adapter);
    
	if (ps != LC->RecvNo)  {
		LC->DiagCode = InValidPs;
		X25ErrorHandler(Adapter,LC->DiagCode,LC);
		//DbgPrint("Packet number error!\n");
        OutX25CriticalRegion(Adapter);
  		return;
	}	// End of if 

	//Processing the acknowledge information 

	if(pr!=LC->SendWin.Low)
	{
		 char ad_ch;
		 int ad_i;
				 
		 /*** adapt send window ***/
		 ad_ch=GetNextWinNo(LC->Modul, LC->SendWin.Low);
		 ad_i=LC->SendQue;

		 if (pr==LC->SendWin.High)
			while (pr!=ad_ch && ad_i!=LAST_PKT) {
				ad_ch=GetNextWinNo(LC->Modul, ad_ch);
				ASSERT((0<=ad_i)&&(ad_i<POOLLEN));
				ad_i=Adapter->Pool[ad_i].Next;
			}	// End of While 
		 else	
			while (pr!=ad_ch && ad_i!=LAST_PKT && ad_ch!=LC->SendWin.High) {
				ch=GetNextWinNo(LC->Modul, ad_ch);
				ASSERT((0<=ad_i)&&(ad_i<POOLLEN));
				ad_i=Adapter->Pool[ad_i].Next;
			}	// End of While 

		 
		 if(pr==ad_ch)
		 {
		 	
			 LC->SendWin.Low=pr;
			 LC->SendWin.Size=GetWinSize(Adapter,&LC->SendWin);
			 LC->TeleBusy=FALSE;
			 
			 /*** insert the confirmated units into system free queue ***/
			 ASSERT((0<=ad_i)&&(ad_i<POOLLEN));
			 ret=Adapter->Pool[ad_i].Next;
			 Adapter->Pool[ad_i].Next=Adapter->SysFreeQue;
			 Adapter->SysFreeQue=LC->SendQue;

			 /*** delete the confirmated units from local send queue ***/
			 LC->SendQue=ret;
			 if (LC->SendQue == LAST_PKT)
			 {
				 LC->SendTail=LAST_PKT;
				 // All packets have been confirmed 
				 // Stop the timer
			 	// NdisReleaseSpinLock(&Adapter->X25LayerLock);
    			// NdisMCancelTimer(&LC->ChannelTimer,&result);
    			// NdisAcquireSpinLock(&Adapter->X25LayerLock);
			 }	 
			// DbgPrint("Data ack,Send window moving\n");
	
			 if (Adapter->init.PktLevState==POOL_OVER)
			     Adapter->init.PktLevState=NORMAL;
			     
        	 OutX25CriticalRegion(Adapter);
    		 NE2000TransmitComplete(Adapter);
    
		 }
		 else  {
		 		   // Error: the receive number is not in the send window.!
		 		   LC->DiagCode=InValidPr;
				   X25ErrorHandler(Adapter,LC->DiagCode,LC);
	               OutX25CriticalRegion(Adapter);
		       }   
	 }
	 else   OutX25CriticalRegion(Adapter);

	 // End of if 



	 
	// Receive the packet from the Link level 
	// and put the packet into the lcn receive queue
	ret=RecvPacket(Adapter,LC, pstr, len, DCE_DATA);
	if (ret < 0)  {
		DbgPrint("Receive Packet:err 70");
		LC->State=LC_ERROR;
		enable();
		return;
	}
	
    EnterX25CriticalRegion(Adapter);
  	LC->RecvNo=GetNextWinNo(LC->Modul, LC->RecvNo);
  
	#ifdef _DEBUG
			RecvCount++;
	#endif
	ASSERT((0<=ret)&&(ret<POOLLEN));
	/* pick out M bit */
	switch(LC->Modul)  {
	case 0x10:
		/* 0x10==0001,0000, is module 8 */
		Adapter->Pool[ret].Mbit=(ch & 0x10) >> 4;
		break;
	case 0x20:
		/* 0x20==0010,0000, is module 128 */
		/* 0x01==0000,0001 */
		Adapter->Pool[ret].Mbit=ch & 0x01;
		break;
	}	// End of case 
					
	M_flag=Adapter->Pool[ret].Mbit;

	ret =0;
	if (Adapter->SysFreeQue != LAST_PKT)
	{
		//DbgPrint(":into  send DTE_rr");
		// Send the Receive_Ready confirmation packet.
		// Commented by Jimmy Guo 
        OutX25CriticalRegion(Adapter);
        // if there is not data frame to pig_back confirmation 
        // then send the receive ready packet to confirm 
   		ret=SendFlowCtlPkt(Adapter,LC, DTE_RR);
		//DbgPrint(":out send DTE_RR");
	}
	else {
        	OutX25CriticalRegion(Adapter);
			DbgPrint(":into send DTE_RNR");
			ret=SendFlowCtlPkt(Adapter,LC, DTE_RNR);
			DbgPrint(":out send DTE_RNR");
		 }
	if (ret < 0) {
			DbgPrint(":err 80");
			LC->State=LC_ERROR;
			enable();
			return;
	}

	//Indicate before send rr Frame.
	if(!M_flag)
	{
		// The operation of dealing with the M_bit
		// This is a  call to upper function to indicate the 
		// Arriving of a packet
		NE2000IndicatePacket(Adapter,LC);
	}
	
	DBG_LEAVE(Adapter);
}

VOID HandleFlowCtlPacket(
	IN PNE2000_ADAPTER    Adapter,
	IN struct PktHead   head,
	IN PUCHAR           Data_buf,
	IN INT              Data_len,
	IN struct LogicalChannel *LC,
	IN UCHAR            Mark,
	IN UCHAR           Pkt_type
)
//************************************************************
// Function to Process the FlowCtl_packet Arrival Event
// Processing include
// 1.     Acknowledge information processing
// 2.     Logical Channel state maintaining
// 3.     REJ packet is not supported here
//************************************************************

{
	u_char  ch, ps, pr, *pstr;
	int     len;
	int     ArrivedPacket;
	int     M_flag;
	int     ret;
	int     i;
	BOOLEAN result;
	
	DBG_FUNC("Handle Data acknowledge packet")
	DBG_ENTER(Adapter);
	
//	Initialization of the variable
//  Begin of section 
	ch = Mark;
	len = Data_len;
	pstr = Data_buf;
	ArrivedPacket = 0;
//  End of section 


	if (LC->State != DATA_TRANS)  {
	// #ifdef _DEBUG
	// 	DbgPrint(":err 90");
	// #endif
	//	LC->State=LC_ERROR;
		return;
	}
	
/* pick out P(R) */
	switch (LC->Modul)  {
	case 0x10:
	/* 0x10==0001 0000, is module 8 */
	/* 0xe0==1110 0000 */
		pr=(ch & 0xe0) >> 5;
		break;
	case 0x20:
	/* 0x20==0010 0000, is module 128 */
 		if (len <4)  
 		{
	 		DbgPrint(":err 100");
			LC->DiagCode=PacketTooShort;
			X25ErrorHandler(Adapter,LC->DiagCode,LC);
			return;
		}
		pr=(*pstr++) >> 1;
		break;
	}    // End of case 


	// Set the link state
   EnterX25CriticalRegion(Adapter);

    if (Pkt_type == DCE_RR)
    	LC->TeleBusy = FALSE;
    else
	    LC->TeleBusy = TRUE;
	    
	if(pr!=LC->SendWin.Low)
	{
	 /*** adapt send window ***/

		ch=GetNextWinNo(LC->Modul, LC->SendWin.Low);
		i=LC->SendQue;
		if (pr==LC->SendWin.High)
			while (pr!=ch && i!=LAST_PKT) {
				ch=GetNextWinNo(LC->Modul, ch);
				ASSERT((0<=i)&&(i<POOLLEN));
				i=Adapter->Pool[i].Next;
			}	// End of While 
		else	
			while (pr!=ch && i!=LAST_PKT && ch!=LC->SendWin.High) {
				ch=GetNextWinNo(LC->Modul, ch);
				ASSERT((0<=i)&&(i<POOLLEN));
				i=Adapter->Pool[i].Next;
			}	// End of While 


 	
		 if(pr==ch) {
			 LC->SendWin.Low=pr;
			 LC->SendWin.Size=GetWinSize(Adapter,&LC->SendWin);
			 
			 /*** insert the confirmated units into system free queue ***/
			 ASSERT((0<=i)&&(i<POOLLEN));
			 ret=Adapter->Pool[i].Next;
			 Adapter->Pool[i].Next=Adapter->SysFreeQue;
			 Adapter->SysFreeQue=LC->SendQue;

			 /*** delete the confirmated units from local send queue ***/
			 LC->SendQue=ret;
			 if (LC->SendQue == LAST_PKT)
			 {
				 LC->SendTail=LAST_PKT;
				 // All packets have been confirmed 
				 // Stop the timer
		 		 // NdisReleaseSpinLock(&Adapter->X25LayerLock);
				 // NdisMCancelTimer(&LC->ChannelTimer,&result);
				 // NdisAcquireSpinLock(&Adapter->X25LayerLock);
				 
			 }			 
			 //DbgPrint("Ack frame,Send window moving\n");

	
			 if (Adapter->init.PktLevState==POOL_OVER) 
			 	Adapter->init.PktLevState=NORMAL;
			 OutX25CriticalRegion(Adapter);

			 if (Pkt_type == DCE_RR	)
			 	NE2000TransmitComplete(Adapter);

//			 NdisReleaseSpinLock(&Adapter->X25LayerLock);

		 }
		 else {
			//	 #ifdef _DEBUG
			 DbgPrint("Error: Invalid pr");
			//			 #endif
			 LC->DiagCode=InValidPr;
			 X25ErrorHandler(Adapter,LC->DiagCode,LC);
	         OutX25CriticalRegion(Adapter);
			 return ;
		  }
	}			  
	else 	OutX25CriticalRegion(Adapter);

}





VOID HandleConnectReqPacket(
	IN PNE2000_ADAPTER    Adapter,
	IN struct PktHead   head,
	IN PUCHAR           Data_buf,
	IN INT              Data_len,
	IN UCHAR            lcn,
	IN UCHAR            Mark
)
//************************************************************
// Function to Process the call_req_packet Arrival Event
// Processing include
// 1.     Receive the   call_req packet
// 2.     Up_load the   call_req to the upper layer(ipx25 router)
//************************************************************

{
	u_char  ch, ps, pr, *pstr;
	u_int   len, k1, k2;
	int        Fd, ret, i;
	PKT_TYP buff;
	struct NetAddr s_addr, d_addr,t_addr;
	struct LogicalChannel *LC;


	DBG_FUNC("HandleConnectReqPacket")
	DBG_ENTER(Adapter);
	
//  Initialization of local variable
	len  = Data_len;	
	ch   = Mark;
	pstr = Data_buf;
	ASSERT((0<=lcn)&&(lcn<VC_NUM));
	LC=&Adapter->Channel[lcn];
	

	

	if (len <4)  {
//		#ifdef _DEBUG
		DbgPrint(":err 150");
//		#endif
		LC->State=LC_ERROR;
		enable();
		return;
	}
	ch=*pstr++;
	len-=4;
	
/* pick out calling address length (in byte) */
	k1=(u_int)((ch & 0xf0) >> 4);
	s_addr.Len=k1;
	k1=(k1/2)+(k1%2);
	
/* pick out called address length (in byte) */
	k2=(u_int)(ch & 0x0f);
	d_addr.Len=k2;
	k2=(k2/2)+(k2%2);
	if (len < (k1+k2+1))  {
//	#ifdef _DEBUG
		DbgPrint(":err 160");
//	#endif
		LC->State=LC_ERROR;
		enable();
		return;
	}
	len-=(k1+k2);

	
	// Processing according to 
	// the state of logical channel 
	switch(LC->State)  {
	case READY:                 /* P1 */
		/*** take called address into d_addr ***/
		StrCopy(d_addr.Addr, pstr, k2);
		pstr+=k2;
		/*** take calling address into s_addr ***/
		StrCopy(s_addr.Addr, pstr, k1);
		pstr+=k1;
		LC->State = DCE_WAIT;		
		addrcpybts(&t_addr,&d_addr);
		// Comparing the x25 Address
		if (IsHostX25Address(t_addr))
		{
			addrcpybts(&t_addr,&s_addr);
			if (!UpLoadCallReq(Adapter,&t_addr,lcn)) {
				LC->State = READY;
				ChannelIssueClearReq(Adapter,LC);
			}	
		}
		else 
		{	
			LC->State = READY;
			ChannelIssueClearReq(Adapter,LC);
		}
		
			break;
	case DTE_WAIT:           /* P2 */
			LC->State=CALL_COLI;    /* P5 */
			break;
	case CLR_REQ:
			break;
	default:
		//#ifdef _DEBUG
			DbgPrint(":err 190");
		//#endif
			LC->State=LC_ERROR;
			break;
	}  //End of case 
	
	DBG_LEAVE(Adapter);
}	



VOID HandleConnectConfPacket(
	IN PNE2000_ADAPTER    Adapter,
	IN struct PktHead   head,
	IN PUCHAR           Data_buf,
	IN INT              Data_len,
	IN UCHAR            lcn,
	IN UCHAR            Mark
)
//************************************************************
// Function to Process the call_conf_packet Arrival Event
// Processing include
// 1.     Receive the   call_conf packet
// 2.     Up_load the   call_conf to the upper layer(ipx25 router)
//************************************************************

{
	u_char  ch, ps, pr, *pstr;
	u_int   len, k1, k2;
	int        Fd, ret, i;
	PKT_TYP buff;
	struct NetAddr s_addr, d_addr;
	struct LogicalChannel *LC;
	BOOLEAN result;


	DBG_FUNC("HandleConnectConfPacket")
	DBG_ENTER(Adapter);
	
//  Initialization of local variable
	len  = Data_len;	
	ch   = Mark;
	pstr = Data_buf;
	
	ASSERT((0<=lcn)&&(lcn<VC_NUM));
	LC=&Adapter->Channel[lcn];

	/*** process call connected packet ***/
	switch(LC->State)  {
		case DTE_WAIT:       /* P2 */
		case CALL_COLI:        /* P5 */
				len-=3;
				if(len==0)
				{
					 len=2;
					 pstr[0]='\0';
					 pstr[1]='\0';
					 pstr[2]='\0';
				}
				ch=*pstr++;
				k1=(u_int)((ch & 0xf0) >> 4);
				k2=(u_int)(ch & 0x0f);
				k1=(k1/2)+(k1%2);
				k2=(k2/2)+(k2%2);
				if (len < (k1+k2+2))  {
					#ifdef _DEBUG
						DbgPrint(":err 195");
					#endif
					LC->State=LC_ERROR;
				}
				else 
				{
					pstr+=(k1+k2);
					len-=(k1+k2+1);
					// Since the call_conf packet is received 
					// Stop  the call timer and fill the map table.
					LC->State=DATA_TRANS;
					//NdisMCancelTimer(&LC->CallTimer,&result);
					UpLoadCallConf(Adapter,LC);
				}
				break;

	}
	DBG_LEAVE(Adapter);

}

VOID HandleClearReqPacket(
	IN PNE2000_ADAPTER    Adapter,
	IN struct PktHead   head,
	IN PUCHAR           Data_buf,
	IN INT              Data_len,
	IN UCHAR            lcn,
	IN UCHAR            Mark
)
//************************************************************
// Function to Process the clear_req_packet Arrival Event
// Processing include
// 1.     Receive the   clear_req packet
// 2.     Up_load the   clear_req to the upper layer(ipx25 router)
//************************************************************
{
	u_char  ch, ps, pr, *pstr;
	u_int   len, k1, k2;
	int        Fd, ret, i;
	PKT_TYP buff;
	struct NetAddr s_addr, d_addr;
	struct LogicalChannel *LC;
	BOOLEAN result;

	DBG_FUNC("HandleClearReqPacket")
	DBG_ENTER(Adapter);
	
//  Initialization of local variable
	len  = Data_len;	
	ch   = Mark;
	pstr = Data_buf;
	
	ASSERT((0<=lcn)&&(lcn<VC_NUM));

	LC=&Adapter->Channel[lcn];

	if (len < 5)  {
		DBG_NOTICE(Adapter,("Packet length error\n"));
		//LC->State=LC_ERROR;
		enable();
		return;
	}
	
	LC->Cause=*pstr++;
	LC->DiagCode=*pstr++;

	switch(LC->State) {
		case CLR_REQ:         /* P6 */
		// The channel is already being cleared 
		// the just initialize the channel.

		// Stop the Clear timer  .
		// NdisMCancelTimer(&LC->CallTimer,&result);
		// Initialize the channel     
			ChannelInitialize(Adapter,lcn);
			break;
		case READY:
		// Clear a ready channel 
		// Then send  the clear_conf packet 
			ChannelIssueClearConf(Adapter,LC);
			break;
		case DTE_WAIT:
		// Clear a Call issuing chaneel 
		// UpLoad the clear_req to the upper IPX25 router
			LC->State = CLR_IND;
			UpLoadClearReq(Adapter,LC);
			break;
		case DATA_TRANS:
		// Clear a Data_Transmitting  issuing chaneel 
		// UpLoad the clear_req to the upper IPX25 router
			LC->State = CLR_IND;
			UpLoadClearReq(Adapter,LC);
			break;	
	}	
}


VOID HandleClearConfPacket(
	IN PNE2000_ADAPTER    Adapter,
	IN struct PktHead   head,
	IN PUCHAR           Data_buf,
	IN INT              Data_len,
	IN UCHAR            lcn,
	IN UCHAR            Mark
)
//************************************************************
// Function to Process the clear_conf_packet Arrival Event
// Processing include
// 1.     Receive the   clear_conf packet
// 2.     Up_load the   clear_conf to the upper layer(ipx25 router)
//************************************************************
{
	u_char  ch, ps, pr, *pstr;
	u_int   len, k1, k2;
	int        Fd, ret, i;
	PKT_TYP buff;
	struct NetAddr s_addr, d_addr;
	struct LogicalChannel *LC;
	BOOLEAN result;

	DBG_FUNC("HandleClearConfPacket")
	DBG_ENTER(Adapter);
	
//  Initialization of local variable
	len  = Data_len;	
	ch   = Mark;
	pstr = Data_buf;

	ASSERT((0<=lcn)&&(lcn<VC_NUM));
	LC=&Adapter->Channel[lcn];

	switch(LC->State) {
		case CLR_REQ:         
		// The clear request is being responsed 
		// Then initialize the channel.

		// Stop the Clear timer  .
		//    NdisMCancelTimer(&LC->CallTimer,&result);
		    
		// Initialize the channel     
			ChannelInitialize(Adapter,lcn);
			break;
	}		
}

	
VOID HtDsuHandlePacket(
	IN PNE2000_ADAPTER    Adapter,
	IN PUCHAR             Data_buf,
	IN UINT               Data_len
    )
{
	//The following section is variable defined in int.c
	//Begin of section 
	u_char  ch, ps, pr, *pstr;
	int   len, k1, k2;
	int        Fd, ret, i;
	PKT_TYP buff;
	struct NetAddr s_addr, d_addr;
	struct PktHead head;
	struct LogicalChannel *LC;
	int   M_flag;
	//End of section 
	
	//Variable Arrivepacket is used to indicate the received packet.
	int   ArrivedPacket;

	DBG_FUNC("HtDsuHandlePacket")
  	DBG_ENTER(Adapter);

	ArrivedPacket =0;
	disable();
	

	pstr = Data_buf;
	len  = Data_len;	

	/* pick out GFI, LCGN and LCN */
	if (len < 3) {
		#ifdef _DEBUG
			DbgPrint("Packet length error!!\n");
		#endif
		//Adapter->init.PktLevState=DATA_ERR;
		enable();
		return;
	}

	ch=*pstr++;
	head.Qbit=(ch & 0x80) >> 7;   /* 0x80==1000,0000 */
	head.Dbit=(ch & 0x40) >> 6;   /* 0x40==0100,0000 */
	head.Modul=ch & 0x30;         /* 0x30==0011,0000 */
	head.LCGN=ch & 0x0f;          /* 0x0f==0000,1111 */
	head.LCN=*pstr++;


	if (!((0<=head.LCN)&&(head.LCN<VC_NUM)))
	// Invalid channel number
		return;
		
	ASSERT((0<=head.LCN)&&(head.LCN<VC_NUM));

	LC=&Adapter->Channel[head.LCN];
	if ((i=CheckPktHead(LC, &head)) < 0)  {
		#ifdef _DEBUG
			DbgPrint("Packet head error!!\n");
		#endif
		//Adapter->init.PktLevState=DATA_ERR;
		return;
	}
	ch=*pstr++;

	
	//  Processing the data packet
	if ((ch & 0x01)==DCE_DATA) 
	{
		HandleDataPacket(Adapter,head,pstr,
						 len,LC,ch);
        return;
    }   

	// Processing the flow control packet
    if (((ch & 0x1f)==DCE_RR) || ((ch & 0x1f)==DCE_RNR))
    {
    	HandleFlowCtlPacket(Adapter,head,pstr,
						 len,LC,ch,(UCHAR)( ch & 0x1f));
		DBG_LEAVE(Adapter);						 
		return;
	}	

	switch(ch) {
		case INCOM_CALL:
			HandleConnectReqPacket(Adapter,head,pstr,
						len,head.LCN,ch);		
		    break;
		case CALL_CONN:
			HandleConnectConfPacket(Adapter,head,pstr,
						len,head.LCN,ch);		
			break;			
		case CLEAR_IND:
			HandleClearReqPacket(Adapter,head,pstr,
						len,head.LCN,ch);
			break;
		case DCE_CLR_CONF:
			HandleClearConfPacket(Adapter,head,pstr,
						len,head.LCN,ch);
			break;
		case RESTART_IND:
			HandleRestartReqPacket(Adapter,head,pstr,
						len,head.LCN,ch);
			break;
		case DCE_RST_CONF:	
			HandleRestartConfPacket(Adapter,head,pstr,
						len,head.LCN,ch);
			break;
		case RESET_IND:
			HandleResetReqPacket(Adapter,head,pstr,
						len,LC,ch);
			break;
		case DCE_RES_CONF:
			HandleResetConfPacket(Adapter,head,pstr,
						len,LC,ch);
			break;
	}	    
	DBG_LEAVE(Adapter);
}


