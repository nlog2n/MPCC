// This file is intended to fullfil the operation of 
// the automation  step by step 
 

#include <iostream.h>                     //for cout and cin
#include <string.h>                       //for  strcat etc.
#include <ndis.h>
#include "mpcccard.h"
#include "ne2000hw.h"
#include "ne2000sw.h"
#include "keywords.h"
#include "mpccfunc.h"
#include "debug.h"
#include "typdef.h"
#include "const.h"
#include "extervar.h"
#include "func.h"

BOOLEAN ChannelIssueCallConf(
					 IN PNE2000_ADAPTER        Adapter,
					 IN int                lcn,
					 IN struct CallStruct    *SendCall);
					 
int ChannelIssueClearConf(
					IN PNE2000_ADAPTER        Adapter,
					IN struct LogicalChannel *lc
					);

int ChannelIssueClearReq(
					IN PNE2000_ADAPTER       Adapter,
					IN struct LogicalChannel *LC
					);
					

VOID  EmulateMulticastPacket(
	IN PNE2000_ADAPTER     Adapter,
    IN UCHAR            SourAddr[4]   //Source Ip Address  
								);

VOID  EmulateAnswerIpPacket(
	IN PNE2000_ADAPTER     Adapter,
    OUT UCHAR           SourAddr[4],   //Query Ip   Address
    OUT UCHAR           DestAddr[4]    //Query Dest Address
	);

					
VOID
NE2000DoNextSend(
    PNE2000_ADAPTER  Adapter,
    IN BOOLEAN       OnePacket,  
    int           RouteItem
    );

VOID
ChannelInitialize(IN PNE2000_ADAPTER        Adapter,
				  IN int    lcn );    

// Variable to record the actual route number in the route table
int RouteItemNumber;


VOID SetTimeStamp(
	IN PNDIS_PACKET 	   Packet		//Send packet
)
{
	PUCHAR 			BufAddress;
	PNDIS_BUFFER 	CurBuffer,NextBuffer;
	int 			CurBufLen=0;
	UINT  			PacketLength;
	int 			Tmp;
	UINT            BytesOk=0;
	UINT            CurBufSend=0;

	NdisQueryPacket(Packet, NULL, NULL, &CurBuffer, &PacketLength);
   	
   	if (CurBuffer)
		NdisQueryBuffer(CurBuffer, (PVOID *)&BufAddress, &CurBufLen);
	
    while (CurBuffer && (CurBufLen == 0)) {
		NdisGetNextBuffer(CurBuffer, &CurBuffer);
		NdisQueryBuffer(CurBuffer, (PVOID *)&BufAddress, &CurBufLen);
    }

	// The packet is empty
	if ((PacketLength==0) ||(CurBufLen==0)||(!CurBuffer))
	  return;

	*BufAddress=0;	
}

UCHAR IncTimeStamp(
	IN PNDIS_PACKET 	   Packet		//Send packet
)
{
	PUCHAR 			BufAddress;
	PNDIS_BUFFER 	CurBuffer,NextBuffer;
	int 			CurBufLen=0;
	UINT  			PacketLength;
	int 			Tmp;
	UINT            BytesOk=0;
	UINT            CurBufSend=0;

	NdisQueryPacket(Packet, NULL, NULL, &CurBuffer, &PacketLength);
   	
   	if (CurBuffer)
		NdisQueryBuffer(CurBuffer, (PVOID *)&BufAddress, &CurBufLen);
	
    while (CurBuffer && (CurBufLen == 0)) {
		NdisGetNextBuffer(CurBuffer, &CurBuffer);
		NdisQueryBuffer(CurBuffer, (PVOID *)&BufAddress, &CurBufLen);
    }

	// The packet is empty
	if ((PacketLength==0) ||(CurBufLen==0)||(!CurBuffer))
	  return MAX_ENDURE_TIME;
	
	*BufAddress=*BufAddress+1;	
	return *BufAddress;
}




VOID X121AddressCopy(
				IN struct NetAddr *sour,
				IN struct NetAddr *dest
)
{
   u_int i;
   dest->Len = sour->Len;
   for (i=0;i<sour->Len;i++)
   	dest->Addr[i]=sour->Addr[i];
}

BOOLEAN IsHostIpAddress(
	IN PUCHAR BufAddress)
{
	return (EqualIpAddress(HostIpAddress,BufAddress));
}


int IsHostX25Address(IN struct NetAddr X25Addr)
{
	
	return(!AddrComp(&X121_Address,&X25Addr));
}



BOOLEAN LookUpIpAddress(
		 IN PNE2000_ADAPTER    Adapter,
		 IN  UCHAR     IpAddress[4],
		 OUT int     *RouteItemNum
)	
{						 	
	UCHAR tmp;
	for (tmp=0;tmp<RouteItemNumber;tmp++)
		if EqualIpAddress(Adapter->RouteTable[tmp].IpAddress,IpAddress) 
			break;
	if (tmp == RouteItemNumber)
	   return FALSE;
	*RouteItemNum = tmp;
	return TRUE;
}

BOOLEAN LookUpX25Address(
	 	IN PNE2000_ADAPTER    Adapter,
		IN  struct NetAddr    X25Address,
		OUT int             *RouteItemNum
)
{
	UCHAR tmp; 
	for (tmp=0;tmp<RouteItemNumber;tmp++)
		if (!AddrComp(&Adapter->RouteTable[tmp].X25Address,&X25Address))
			break;
	if (tmp == RouteItemNumber)
	   return FALSE;
	*RouteItemNum = tmp;
	return TRUE;
	
}


BOOLEAN ChannelStatusOn(
					IN PNE2000_ADAPTER    Adapter,
					IN  int    lcn)			
{
	if (lcn==-1)
		return FALSE;
	if ((lcn<Adapter->init.LTC)&&(lcn>Adapter->init.HOC))
		return FALSE;
	return((Adapter->Channel[lcn].State==DATA_TRANS));	
}


VOID UpLoadRestartReq(
		IN PNE2000_ADAPTER    Adapter
)
{
	USHORT tmp;
	USHORT j;
	PTableItem      RouteEntry;	   // Route table entry
	PNDIS_PACKET    Packet;       // The packet to process.
	BOOLEAN         result;



	EnterIpRouterCriticalRegion(Adapter);
	Adapter->NotFirstFrame = FALSE ;
	for (tmp=0;tmp<RouteItemNumber;tmp++)
	{
		
		Adapter->RouteTable[tmp].FirstPacket     = NULL;
		Adapter->RouteTable[tmp].LastPacket      = NULL;
		Adapter->RouteTable[tmp].BindChannel     = -1;
		Adapter->RouteTable[tmp].LinkCallState   = FALSE;
		Adapter->RouteTable[tmp].QueryRemote     = FALSE;
		Adapter->RouteTable[tmp].BytesSend       = NE2000_HEADER_SIZE;
		
		// Indicate the failure of the transmisson  is needed
		RouteEntry = &Adapter->RouteTable[tmp];

		NdisMCancelTimer(&RouteEntry->IdleTimer,&result);


		while (RouteEntry->FirstPacket != NULL)  {
			Packet = RouteEntry->FirstPacket;
			NdisMSendComplete(
                Adapter->MiniportAdapterHandle,
                Packet,
                NDIS_STATUS_FAILURE
                );
	        RouteEntry->FirstPacket = RESERVED(Packet)->Next;
		    if (Packet == RouteEntry->LastPacket) 
  		      RouteEntry->LastPacket = NULL;
		}

		
	}	
	OutIpRouterCriticalRegion(Adapter);

	//NdisReleaseSpinLock(&Adapter->Lock);

}

BOOLEAN UpLoadCallReq(
		IN PNE2000_ADAPTER    Adapter,
		IN  struct NetAddr *s_addr,
		IN  int    lcn
)
{
	struct CallStruct IssueCall;
	int  RouteNum;
	struct LogicalChannel *LC;
	
	ASSERT((0 < lcn)&&(lcn<VC_NUM));
	LC = &Adapter->Channel[lcn];

	if (LookUpX25Address(Adapter,*s_addr,&RouteNum))
	{
		// Indicate the remote Ip address to the upper level
		EmulateMulticastPacket(Adapter,Adapter->RouteTable[RouteNum].IpAddress);
	
		IssueCall.Opt.Len  = 0;
		IssueCall.Data.Len = 1;
		IssueCall.Data.Buf[0]=0xcc;
		X121AddressCopy(&Adapter->RouteTable[RouteNum].X25Address,&IssueCall.Addr);
		if	(ChannelIssueCallConf(Adapter,lcn,&IssueCall))
		{
			Adapter->RouteTable[RouteNum].BindChannel = lcn;
			LC->Fd=RouteNum;
			return TRUE;
		}
	}	
	return FALSE;
}

VOID UpLoadCallConf(
		IN PNE2000_ADAPTER    Adapter,
		IN struct LogicalChannel *LC)
{
	int RouteItem;

	RouteItem = LC->Fd;
	Adapter->RouteTable[RouteItem].BindChannel = LC->LCN;
	Adapter->RouteTable[RouteItem].LinkCallState   = FALSE;  
	if (Adapter->RouteTable[RouteItem].QueryRemote)
	{
		Adapter->RouteTable[RouteItem].QueryRemote= FALSE;
		EmulateAnswerIpPacket(Adapter,HostIpAddress,Adapter->RouteTable[RouteItem].IpAddress);		
	}
	NE2000DoNextSend(Adapter,FALSE,RouteItem);
}

	
VOID UpLoadClearReq(
	IN PNE2000_ADAPTER    Adapter,
	IN struct LogicalChannel *LC		
)
{
	int 			RouteItem;
    PNDIS_PACKET    Packet;       // The packet to process.
	PTableItem      RouteEntry;	   // Route table entry
	BOOLEAN    		result;


	RouteItem= LC->Fd;
	if ((0<=RouteItem)&&(RouteItem<RouteItemNumber))
	{
		RouteEntry = &Adapter->RouteTable[RouteItem];
	
		RouteEntry->BindChannel = -1;
		RouteEntry->LinkCallState = FALSE;
		RouteEntry->QueryRemote   = FALSE;

		NdisMCancelTimer(&RouteEntry->IdleTimer,&result);
	
		//  Indicate the send failure to the upper layer.
		//  Since the lower channel is cleared 
		while (RouteEntry->FirstPacket != NULL)  {
			Packet = RouteEntry->FirstPacket;
			NdisMSendComplete(
                Adapter->MiniportAdapterHandle,
                Packet,
                NDIS_STATUS_FAILURE
                );
        RouteEntry->FirstPacket = RESERVED(Packet)->Next;
	    if (Packet == RouteEntry->LastPacket) 
  		      RouteEntry->LastPacket = NULL;
	 }
	} 

	// The channel is cleared  
	// Stop the  timer belong to the channel 
	if (LC->SendQue != LAST_PKT)
	{
		//NdisMCancelTimer(&LC->CallTimer,&result);
	}
	ChannelIssueClearConf(Adapter,LC);
	ChannelInitialize(Adapter,LC->LCN);
}
				   

VOID UpLoadResetReq(
		IN PNE2000_ADAPTER    Adapter,
		IN struct LogicalChannel *LC)
{
	int 			RouteItem;
	PTableItem      RouteEntry;	  // Route table entry
	PNDIS_PACKET    Packet;       // The packet to process.
	BOOLEAN         result;


	EnterIpRouterCriticalRegion(Adapter);

	RouteItem= LC->Fd;


	if ((0<=RouteItem)&&(RouteItem < RouteItemNumber))
	{
		RouteEntry = &Adapter->RouteTable[RouteItem];


		// Initialize the RouteTable Item associating with the
		// logical channel 
		RouteEntry->BindChannel     = -1;
		RouteEntry->LinkCallState   = FALSE;
		RouteEntry->BytesSend       = NE2000_HEADER_SIZE;
		RouteEntry->QueryRemote     = FALSE;
		NdisMCancelTimer(&RouteEntry->IdleTimer,&result);

	
		// Indicate the failure of the transmisson  is needed
		// The packet is lost in the network.
		while (RouteEntry->FirstPacket != NULL)  {
			Packet = RouteEntry->FirstPacket;
			NdisMSendComplete(
               Adapter->MiniportAdapterHandle,
               Packet,
               NDIS_STATUS_SUCCESS
               );
	        RouteEntry->FirstPacket = RESERVED(Packet)->Next;
		    if (Packet == RouteEntry->LastPacket) 
  			      RouteEntry->LastPacket = NULL;
		}
	 }	
	OutIpRouterCriticalRegion(Adapter);

}


VOID 
LapbReady(
	IN PNE2000_ADAPTER    Adapter
)
{
	// Lapb layer is ready
	// Initialize the packet level variable
	// Reset the logical channel 0
	DBG_FUNC("LapbReady")
    DBG_ENTER(Adapter);
	if (Adapter->PktLevelInitialized)
	{
		Adapter->init.PktLevState=NORMAL;
		SendRestartPkt(Adapter,RESTART_REQ,0,0);
	}	
    DBG_LEAVE(Adapter);
	
}	


VOID
IpTimerHandler(
    IN PVOID                  SystemSpecific1,
    PNE2000_ADAPTER  			  Adapter,
    IN PVOID                  SystemSpecific2,
    IN PVOID                  SystemSpecific3
    )
{
	PTableItem RouteEntry;
	PNDIS_PACKET Packet;
	USHORT       tmp;
	UCHAR        TimeElapsed;

	for (tmp=0;tmp<RouteItemNumber;tmp++)
	{
		RouteEntry = &Adapter->RouteTable[tmp];
		

		Packet = RouteEntry->FirstPacket;
			/*RouteEntry->RetryTime ++;
			if (RouteEntry->RetryTime >= MAX_ENDURE_TIME)
			{
					DbgPrint("Now Discard packet!\n");
					RouteEntry->RetryTime =0;*/
		while (Packet!= NULL)
		{
			TimeElapsed = IncTimeStamp(Packet);
			if (TimeElapsed >= MAX_ENDURE_TIME) {
				   NdisMSendComplete(
        	    	    Adapter->MiniportAdapterHandle,
		                Packet,
    	   		        NDIS_STATUS_SUCCESS
	                );
				   RouteEntry->FirstPacket = RESERVED(Packet)->Next;
		  		   if (Packet == RouteEntry->LastPacket) 
  			    		  RouteEntry->LastPacket = NULL;
				   RouteEntry->BytesSend=NE2000_HEADER_SIZE;    
				   Adapter->FramesXmitGood++;
		 	}
		 	Packet = RESERVED(Packet)->Next;
		 }
	}
	NdisMSetTimer(&Adapter->IpTimer,Monitor_Interval);  


}		
	


VOID
IpIdleTimerHandler(
    IN PVOID                  SystemSpecific1,
	PTableItem 				  RouteEntry,
    IN PVOID                  SystemSpecific2,
    IN PVOID                  SystemSpecific3
    )
{
    PNE2000_ADAPTER  			  Adapter;
	struct LogicalChannel 	  *LC;
	PNDIS_PACKET Packet;
	USHORT       tmp;
	UCHAR        TimeElapsed;
	int          result;

	Adapter = (PNE2000_ADAPTER )RouteEntry->NE2000Adapter;
	LC=&(Adapter->Channel[RouteEntry->BindChannel]);
	result = ChannelIssueClearReq(Adapter ,LC);
	if (result>=0)
	{
		// channel is cleared, then 
		// reset the RouteEntry structure
		// RouteEntry->IdleTime    = 0;
		RouteEntry->BindChannel = -1;
		RouteEntry->LinkCallState = FALSE;
		RouteEntry->QueryRemote   = FALSE;
		RouteEntry->BytesSend     = NE2000_HEADER_SIZE;
		while (RouteEntry->FirstPacket != NULL)  {
			Packet = RouteEntry->FirstPacket;
			NdisMSendComplete(
		        Adapter->MiniportAdapterHandle,
                Packet,
   				NDIS_STATUS_SUCCESS
            );
	        RouteEntry->FirstPacket = RESERVED(Packet)->Next;
		    if (Packet == RouteEntry->LastPacket) 
		  		      RouteEntry->LastPacket = NULL;
			 }
	}
	else   NdisMSetTimer(&RouteEntry->IdleTimer,Max_Idle_Time);  

	  
}

VOID
NE2000TimerHandler(
    IN PVOID                  SystemSpecific1,
    PNE2000_ADAPTER  			  Adapter,
    IN PVOID                  SystemSpecific2,
    IN PVOID                  SystemSpecific3
    )
{
    BOOLEAN        result;
    int            i;
    struct LogicalChannel *LC;
					

	DBG_FUNC("NE2000TimerHandler")
//DBG_ENTER(Adapter->init.Adapter);
	    
//  Adapter = Adapter->init.Adapter;
//	NdisAcquireSpinLock(&Adapter->Lock);

	
	switch(LC->State) {
	 case DATA_TRANS:
	 	// Some packet is not acknowledged on time
	 	// Resend the packet queued in the lcn 
	 	// Local transmit queue.
		DBG_NOTICE(Adapter,("Enter data packet timeout\n")); 	
	 	i=ReSend(Adapter,LC);
	 	// Restart the timer
//		NdisMCancelTimer(&LC->CallTimer,&result);
//		NdisMSetTimer(&LC->CallTimer,DATA_PACKET_TIMEOUT);
		break;
		
	 case DTE_WAIT:
	 	// Call time_out
	 	DBG_NOTICE(Adapter,("Enter Call_Req packet timeout\n")); 	
	 	// Stop the call timer
		// NdisMCancelTimer(&LC->CallTimer,&result);
		// Call time_out equals to receive a clear_req packet
		UpLoadClearReq(Adapter,LC);
	 	break;
	 case CLR_REQ:
	 	// Clear time_out 
		DBG_NOTICE(Adapter,("Enter Clear_Req packet timeout\n")); 	
		
		// Stop the clear timer
		//	NdisMCancelTimer(&LC->CallTimer,&result);
		// Initialize the channel 	
	 	ChannelInitialize(Adapter,LC->LCN);
	 	break;
	  }  
//	NdisReleaseSpinLock(&Adapter->Lock);
//	DBG_LEAVE(Adapter->init.Adapter);

}

void InitializeRouteTable(
		 IN PNE2000_ADAPTER    Adapter,
		 IN PADDR_PAIR_STRING    StringBuf
						)
{

	USHORT tmp;
	USHORT j;
	int    RouteItem=0;
	PTableItem RouteEntry;
	char   IpAddress[4];
	char   X121Address[15];
	int    IpAddressOffset= 0,X121Offset;
	int    CharPointer = 0;
	int    MaxLength;
	char   AddressVal;
	PUCHAR CharBuf;

	CharBuf = StringBuf->Buffer;
	MaxLength = StringBuf->Length;
	
	while (CharPointer<MaxLength)
	{
		if (RouteItem>=Max_Ip_Num)	break;
			
		RouteEntry = &Adapter->RouteTable[RouteItem];
		
		// get symbol "+"
		while ((CharBuf[CharPointer]!='+')&&(CharPointer<MaxLength))
			CharPointer++;
	
		CharPointer++;

		// Get the IpAddress
		IpAddressOffset = 0;

		while (IpAddressOffset <4)
		{
			while (((CharBuf[CharPointer]>'9')||(CharBuf[CharPointer]<'0'))&&(CharPointer<MaxLength))
				CharPointer++;
			AddressVal = 0;
			while ((CharBuf[CharPointer]>='0')&&(CharBuf[CharPointer]<='9')&&(CharPointer<MaxLength)) 
			{
				AddressVal = AddressVal*10 + CharBuf[CharPointer]-'0';
				CharPointer++;
     		}
			
			RouteEntry->IpAddress[IpAddressOffset++]=AddressVal;
		}

		// Get X25address

		while (((CharBuf[CharPointer]>'9')||(CharBuf[CharPointer]<'0'))&&(CharPointer<MaxLength))
				CharPointer++;
		X121Offset = 0;
		while ((CharBuf[CharPointer]>='0')&&(CharBuf[CharPointer]<='9')&&(CharPointer<MaxLength))
		{
			RouteEntry->X25Address.Addr[X121Offset++] = CharBuf[CharPointer++];
		}
		RouteEntry->X25Address.Len = X121Offset;

		// Initialize the variable in the RouteTable Item 

		RouteEntry->BindChannel     = -1;
		RouteEntry->LinkCallState   = FALSE;
		RouteEntry->FirstPacket     = NULL;
		RouteEntry->LastPacket      = NULL;
		// Omit the transmission of the 802.3 header
		RouteEntry->BytesSend       = NE2000_HEADER_SIZE;
		//Adapter->RouteTable[tmp].BytesSend       = 0;
		RouteEntry->Priorty         = 0;
		//RouteEntry->IdleTime       = 0;
		RouteEntry->NE2000Adapter    = Adapter;

		NdisMInitializeTimer(
       	 	&RouteEntry->IdleTimer,
		     Adapter->MiniportAdapterHandle,
    		 IpIdleTimerHandler,
	    	 RouteEntry
		);

		
		RouteItem++;
	}

	RouteItemNumber = RouteItem;

	
	
}	






/********************************************************************
 NAME       : ChannelInitialize

 DESCRIPTION:
		 The function is to initalize the channel information
 PARAMETERS :
		 (1)lcn: Logical channel number
 RETURN VALUE:
		 None
 *********************************************************************/

VOID
ChannelInitialize(IN PNE2000_ADAPTER        Adapter,
				  IN int    lcn )
{
  struct LogicalChannel *LC;


  ASSERT((0<=lcn)&&(lcn<VC_NUM));

  LC=&Adapter->Channel[lcn];
  LC->LCGN=0;
  LC->LCN=lcn;
  LC->State=READY;
  LC->Cause=LC->DiagCode='\0';
  LC->Dbit='\0';
  LC->Modul=Adapter->init.Modul;
  LC->RecvNo=LC->SendNo=0;
  if (LC->SendQue != LAST_PKT)
  {
  	ASSERT((0<=LC->SendTail)&&(LC->SendTail<POOLLEN));
	Adapter->Pool[LC->SendTail].Next=Adapter->SysFreeQue;
	Adapter->SysFreeQue=LC->SendQue;
	if (Adapter->init.PktLevState==POOL_OVER) Adapter->init.PktLevState=NORMAL;
  }
  LC->SendQue=LC->SendTail=LAST_PKT;
  if (LC->RecvQue != LAST_PKT)
  {
	Adapter->Pool[LC->RecvTail].Next=Adapter->SysFreeQue;
	Adapter->SysFreeQue=LC->RecvQue;
	if (Adapter->init.PktLevState==POOL_OVER) Adapter->init.PktLevState=NORMAL;
  }
  LC->RecvQue=LC->RecvTail=LAST_PKT;
  LC->SendWin.Low=LC->SendWin.High=0;		
  LC->SendWin.Size=0;
  LC->Expedited=LC->TeleBusy=FALSE;
  LC->Fd=-1;
  LC->tcp_len=0;
  }


VOID PacketLevelInitialize(IN PNE2000_ADAPTER   Adapter)
{

	int  Tmp;
	int i;
	
 	Adapter->init.PktLevState = PktLelNotReady;
	Adapter->init.WinSize     = 7;		  //Window size 7.
	Adapter->init.PackSize    = 128;
	Adapter->init.PutSize     = 0;
	Adapter->init.Modul       = 0x10;      // Modul 8  0x10  Modul 128 0x20
	Adapter->init.LIC         = 0;
	Adapter->init.HIC         = 0;
	Adapter->init.LTC         = 1;
	Adapter->init.HTC         = VC_NUM;
	Adapter->init.LOC         = 1;
	Adapter->init.HOC         = VC_NUM;

    X121AddressCopy(&X121_Address,&Adapter->init.DteAddress);
	for (Tmp=1;Tmp<VC_NUM;Tmp++)   {
	     Adapter->Channel[Tmp].SendQue=Adapter->Channel[Tmp].SendTail=LAST_PKT;
/*	     NdisMInitializeTimer(
       	 	&Adapter->Channel[Tmp].CallTimer,
		     Adapter->MiniportAdapterHandle,
    		 NE2000TimerHandler,
	    	 &Adapter->Channel[Tmp]
	  );*/
	}	  

    NdisMInitializeTimer(
       	 	&Adapter->IpTimer,
		     Adapter->MiniportAdapterHandle,
    		 IpTimerHandler,
	    	 Adapter
	  );
	  
	// Set time interval 500ms  
	NdisMSetTimer(&Adapter->IpTimer,Monitor_Interval);  

	for (Tmp=0;Tmp<VC_NUM;Tmp++)
		ChannelInitialize(Adapter,Tmp);	
	
	Adapter->SysFreeQue=0;
	Adapter->CardTransQue=Adapter->CardTransTail = LAST_PKT;
	
	for (Tmp=0; Tmp<POOLLEN; Tmp++)
		Adapter->Pool[Tmp].Next=Tmp+1;
		
	Adapter->Pool[POOLLEN-1].Next=LAST_PKT;

	
//	InitializeRouteTable(Adapter);	
	Adapter->PktLevelInitialized= TRUE;
	//if  (Adapter->pc2card_scbp->link_state[0]==LINK_NORMAL)
	//	LapbReady(Adapter);
	
	if (LookUpX25Address(Adapter,X121_Address,&Tmp))
	{
		for (i=0;i<4;i++)
		{
			HostIpAddress[i]=Adapter->RouteTable[Tmp].IpAddress[i];
		}
	}
	// Allocate the spin lock the adapter uses.
	NdisAllocateSpinLock(&Adapter->Lock);	
	NdisAllocateSpinLock(&Adapter->CardTransmitLock);
	NdisAllocateSpinLock(&Adapter->X25LayerLock);
	NdisAllocateSpinLock(&Adapter->IpRouterLock);
	NdisAllocateSpinLock(&Adapter->QueueTransmitLock);
	NdisAllocateSpinLock(&Adapter->IdleQueueLock);	


}

//*****************************************************
// 	Name:
//		ChannelAllocate
// 	Description:
//  	 Fuction to allocate a free logical channel
// 	Return value:
//   	>0 : Allocate succefully
//	 	<0 : Allocate failure
//*****************************************************

int  ChannelAllocate(IN PNE2000_ADAPTER   Adapter )
{
	int lcn;


	
	for(lcn=Adapter->init.HOC;lcn>=Adapter->init.LTC;lcn--)  {
		if (Adapter->Channel[lcn].State==READY)
			break;
	}

	if (lcn < Adapter->init.LTC)
		return PktLelBusy;
	ChannelInitialize(Adapter,lcn)	;
	return lcn;
}


//*****************************************************
// 	Name:
//		ChannelCallStateHandler
// 	Description:
//  	 Fuction to issue deal with the state transformation 
//		 in the process of primitive N_CONNECT
//	Return value:
//   	>=0 : Legal state transformation 
//	 	<0 :  Otherwise
//*****************************************************
int ChannelCallStateHandler(
					 IN PNE2000_ADAPTER        Adapter,
					 IN UCHAR                lcn,
					 IN int                  event)
{
	struct LogicalChannel *lc;

	lc = &Adapter->Channel[lcn];
	return 1;
}
	

//*****************************************************
// 	Name:
//		ChannelIssueCallReq
// 	Description:
//  	 Fuction to issue a Call_req packet to the remote 
//		 Dte.
//	     This function implement the function of network 
//		 Primitive N_CONNECT.req
// 	Return value:
//   	TRUE: packet is sent out successfully
//	 	FALSE : fail to send out the packet
//*****************************************************

BOOLEAN ChannelIssueCallReq(
					 IN PNE2000_ADAPTER        Adapter,
					 IN int                lcn,
					 IN struct CallStruct    *SendCall)
{
	int ret;
	struct Packet pac;
	struct PoolUnit pktstr;
	struct LogicalChannel *lc;
	struct NetAddr        net_addr;

	ASSERT((0<=lcn)&&(lcn<VC_NUM));
	lc = &Adapter->Channel[lcn];
	
	// Prepare the call_req Packet
	pac.Type=CALL_REQ ;
	pac.Head.Qbit=0;
	pac.Head.Dbit=lc->Dbit;
	pac.Head.Modul=lc->Modul;
	pac.Head.LCGN=lc->LCGN;
	pac.Head.LCN=lcn;

	// Fill in the address Field
	addrcpystb(&(net_addr),&Adapter->init.DteAddress);
	AddrCopy(&pac.Body.Call.CallAddr, &net_addr);
	addrcpystb(&(net_addr),&SendCall->Addr);
	AddrCopy(&pac.Body.Call.CalledAddr,&net_addr);
	optcpy(&(pac.Body.Call.Opt),&(SendCall->Opt));
	datacpy(&(pac.Body.Call.Data),&(SendCall->Data));

	// Send the packet to the  remote DTE
	BindPacket(&pac,&pktstr);              /* bind packet  */
	disable();
	ret=SendToLink(Adapter,&pktstr);
	enable();

	if (ret<0)
	{
		DbgPrint("Fail to send the call_req packet\n");
		return FALSE;
	}
	
//  Start the system timer 
//  The  state of logical channel is modified in 
//  Function SendtoLink
//  Normally it is READY -----> DTE_WAITING
//	NdisMSetTimer(&lc->CallTimer,CALL_TIMEOUT);	
	return TRUE;		
}

//*****************************************************
// 	Name:
//		ChannelIssueCallConf
// 	Description:
//  	 Fuction to issue a Call_Conf packet to the remote 
//		 Dte.
//	     This function implement the function of network 
//		 Primitive N_CONNECT.conf
// 	Return value:
//   	TRUE  : packet is sent out successfully
//	 	FALSE : failure to send out the packet
//*****************************************************

BOOLEAN ChannelIssueCallConf(
					 IN PNE2000_ADAPTER        Adapter,
					 IN int                lcn,
					 IN struct CallStruct    *SendCall)
{
	int ret;
	struct Packet pac;
	struct PoolUnit pktstr;
	struct LogicalChannel *lc;
	struct NetAddr        net_addr;

	ASSERT((0<=lcn)&&(lcn<VC_NUM));
	lc = &Adapter->Channel[lcn];
	
	// Prepare the call_req Packet
	pac.Type=CALL_CONN;
	pac.Head.Qbit=0;
	pac.Head.Dbit=lc->Dbit;
	pac.Head.Modul=lc->Modul;
	pac.Head.LCGN=lc->LCGN;
	pac.Head.LCN=lcn;
	// Fill in the address Field
	// The Address field is needed to be checked.
	addrcpystb(&(net_addr),&Adapter->init.DteAddress);
	AddrCopy(&pac.Body.Call.CallAddr, &net_addr);
	addrcpystb(&(net_addr),&SendCall->Addr);
	AddrCopy(&pac.Body.Call.CalledAddr,&net_addr);
	optcpy(&(pac.Body.Call.Opt),&(SendCall->Opt));
	datacpy(&(pac.Body.Call.Data),&(SendCall->Data));

	// Send the packet to the  remote DTE
	BindPacket(&pac,&pktstr);              /* bind packet  */
	disable();
	ret=SendToLink(Adapter,&pktstr);
	enable();

	if (ret<0)
	{
		DbgPrint("Fail to send the call_req packet\n");
		return FALSE;
	}

	return TRUE;		
}

//*****************************************************
// 	Name:
//		ChannelIssueResetReq
// 	Description:
//  	 Fuction to issue a reset_req packet to the remote 
//		 Dte.
//	     This function implement the function of network 
//		 Primitive N_RESET.Req
// 	Return value:
//   	>=0 : packet is sent out successfully
//	 	<0 : fail to send out the packet
//*****************************************************

int ChannelIssueResetReq(
					IN PNE2000_ADAPTER           Adapter,
					IN struct LogicalChannel  *lc,
					UCHAR                     Cause,
					UCHAR                     Code
					)
{
	int ret;
	struct Packet pac;
	struct PoolUnit pktstr;

	ret=SendResetPkt(Adapter,lc->LCN,RESET_REQ,Cause,Code);
	return ret;
}	

//*****************************************************
// 	Name:
//		ChannelIssueResetConf
// 	Description:
//  	 Fuction to issue a reset_conf packet to the remote 
//		 Dte.
//	     This function implement the function of network 
//		 Primitive N_RESET.conf
// 	Return value:
//   	>=0 : packet is sent out successfully
//	 	<0 : fail to send out the packet
//*****************************************************

int ChannelIssueResetConf(
					IN PNE2000_ADAPTER           Adapter,
					IN struct LogicalChannel  *lc,
					UCHAR                     Cause,
					UCHAR                     Code
					)
{
	int ret;
	struct Packet pac;
	struct PoolUnit pktstr;

	ret=SendResetPkt(Adapter,lc->LCN,DTE_RES_CONF,Cause,Code);
	return ret;
}	



//*****************************************************
// 	Name:
//		ChannelIssueClearReq
// 	Description:
//  	 Fuction to issue a Clear_req packet to the remote 
//		 Dte.
//	     This function implement the function of network 
//		 Primitive N_DISCONNECT.req
// 	Return value:
//   	>=0 : packet is sent out successfully
//	 	<0 : fail to send out the packet
//*****************************************************

int ChannelIssueClearReq(
					IN PNE2000_ADAPTER        Adapter,
					IN struct LogicalChannel *LC
					)
{
	struct Packet pac;
	struct PoolUnit pktstr;
	int    ret;
			
	pac.Type=CLEAR_REQ;
	pac.Head.Qbit=0;
	pac.Head.Dbit=LC->Dbit;
	pac.Head.Modul=LC->Modul;
	pac.Head.LCGN=LC->LCGN;
	pac.Head.LCN=LC->LCN;
	pac.Body.Clear.Cause=LC->Cause;
	pac.Body.Clear.DiagCode=LC->DiagCode;

	BindPacket(&pac,&pktstr);              /* bind packet  */
	ret=SendToLink(Adapter,&pktstr);      /* send packet  */
	if (ret < 0)  {
		DbgPrint(":Packet send Error");
		enable();
		return -1;
	}
/*	if (LC->State == CLR_REQ)
	    NdisMSetTimer(
			&LC->CallTimer,
			CLEAR_TIMEOUT		
		);	*/

	return 0;

}

//*****************************************************
// 	Name:
//		ChannelIssueClearConf
// 	Description:
//  	 Fuction to issue a Clear_req packet to the remote 
//		 Dte.
//	     This function implement the function of network 
//		 Primitive N_DISCONNECT.conf
// 	Return value:
//   	>=0 : packet is sent out successfully
//	 	<0 : fail to send out the packet
//*****************************************************

int ChannelIssueClearConf(
					IN PNE2000_ADAPTER        Adapter,
					IN struct LogicalChannel *lc
					)
{
	int ret;
	struct Packet pac;
	struct PoolUnit pktstr;

	pac.Type=DTE_CLR_CONF;
	pac.Head.Qbit=0;
	pac.Head.Dbit=lc->Dbit;
	pac.Head.Modul=lc->Modul;
	pac.Head.LCGN=lc->LCGN;
	pac.Head.LCN=lc->LCN;

	optcpy(&(pac.Body.Clear.Opt), NULL);
	datacpy(&pac.Body.Clear.Data, NULL);

	BindPacket(&pac,&pktstr);              /* bind packet  */
	ret=SendToLink(Adapter,&pktstr);      /* send packet  */
	if (ret < 0)     {
	//#ifdef _DEBUG
			DbgPrint(":err 1560");
	//#endif
			enable();
			return -1;
	}			
	return 0;
}	

void RestartPacketLevel(
	 IN PNE2000_ADAPTER        Adapter
	)
{
	 u_char ch;
 	 int i;

 	 DbgPrint("Enter Restart Packet level");

	 Adapter->init.PktLevState=NORMAL;
	 
	 //Adapter->init.PktLevState=	PktLelNotReady ;

	 EnterX25CriticalRegion(Adapter);

	 /*** fd array & channel initalization.   ***/
	 for(ch=0;ch<VC_NUM;ch++)    
		ChannelInitialize(Adapter,ch);

	/*** system queues & buffer pool Adapter->init ***/
	 Adapter->SysFreeQue=0;
	 
	 for (i=0; i<POOLLEN; i++)
		 Adapter->Pool[i].Next=i+1;
	 Adapter->Pool[POOLLEN-1].Next=LAST_PKT;

	 OutX25CriticalRegion(Adapter);

// Initialize the Local transmit queue.	 
	 
	 
	 UpLoadRestartReq(Adapter);

	 DbgPrint("<<<Restart packet level");

}


//*****************************************************
// 	Name:
//		HandleInconsistPacketNum   
// 	Description:
//  	 Fuction to Handle the packet number error 
//		 happend on the packet level 
//*****************************************************
VOID HandleIncosistentPacketNum(
			IN PNE2000_ADAPTER           Adapter,
			IN UCHAR ErrorCode,
			IN struct LogicalChannel  *LC
)
{
	// Since packet number is wrong,reset the corresponding 
	// logical channel
	ChannelIssueResetReq(Adapter,LC,
						 0,// DTE originated reset 
						 ErrorCode);
	// Upload the event to the IpX25 Router					 
	UpLoadResetReq(Adapter,LC);						 
	
}

//*****************************************************
// 	Name:
//		HandleInvalidPacketLength   
// 	Description:
//  	 Fuction to Handle the packet length error 
//		 happend on the packet level 
//*****************************************************
VOID HandleInvalidPacketLength(
			IN PNE2000_ADAPTER           Adapter,
			IN UCHAR ErrorCode,
			IN struct LogicalChannel  *LC
)
{
		switch(LC->State) {
	case FLOW_CTL:       /* D1 */
		// InvalidLength when FLOW_CTL
		// Channel is abnormal, then  reset it 
		// Since packet length is wrong,reset the corresponding 
		// logical channel
		ChannelIssueResetReq(Adapter,LC,
						 0,// DTE originated reset 
						 ErrorCode);
		// Upload the event to the IpX25 Router					 
		UpLoadResetReq(Adapter,LC);						 
		break;
	}
}


//*****************************************************
// 	Name:
//		X25ErrorHandler   
// 	Description:
//  	 Fuction to Handle the error happened on the 
//		 X25 packet level 
//*****************************************************

VOID X25ErrorHandler(
	IN PNE2000_ADAPTER       		Adapter,
	IN UCHAR 			  		ErrorCode,
	IN struct LogicalChannel    *LC
)
{
	switch(ErrorCode) {
	case InValidPs:
	case InValidPr: 
	// Invalid packet receive and send number 
	// Handle error happened when transmitting data 
		HandleIncosistentPacketNum(Adapter,ErrorCode,LC);
		break;
	case PacketTooShort:
	case PacketTooLong:
		HandleInvalidPacketLength(Adapter,ErrorCode,LC);
		break;
	}
}

