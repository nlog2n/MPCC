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
#include "channel.h"

VOID SetTimeStamp(IN PNDIS_PACKET 	   Packet);


extern int RouteItemNumber;

VOID  EmulateMulticastPacket(
	IN PNE2000_ADAPTER    Adapter,
    IN UCHAR            SourAddr[4]   //Source Ip Address  
								)
{
	UINT tmp;
	UCHAR TmpBuf[] =
		{ 0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,
		  0x00,0x00,0x00,0x00,			
		  0x00,0x01,0x08,0x00,0x06,0x04,0x00,0x01,0x00,0x00,
		  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} ;       


	// Fill in the destination and source address		  
	DbgPrint("Emulate Ip Address:");
	for (tmp=0;tmp<4;tmp++)		   {
		DbgPrint("%d.",SourAddr[tmp]);
		TmpBuf[NE2000_HEADER_SIZE+10+tmp]= SourAddr[tmp];
		TmpBuf[NE2000_HEADER_SIZE+14+tmp]= SourAddr[tmp];
		TmpBuf[NE2000_HEADER_SIZE+24+tmp]= SourAddr[tmp];
		//TmpBuf[NE2000_HEADER_SIZE+24+tmp]= SourAddr[tmp];
	}	
	// Fill in the header
	for (tmp =0;tmp <6;tmp++) {
		// Destination ethernet address
		//TmpBuf[tmp]=((tmp<2)?0:DestAddr[tmp-2]);
		// Source ethernet address
		TmpBuf[tmp+6]=((tmp<2)?0:SourAddr[tmp-2]);
	}	
	// Do not ask me why ,just fill it
	TmpBuf[12]=0x08;
	TmpBuf[13]=0x06;
	
	if (!Adapter->NotFirstFrame)
		Adapter->NotFirstFrame = TRUE;

	// Indicate the answer frame 
	 NdisMEthIndicateReceive(
                Adapter->MiniportAdapterHandle,
                (NDIS_HANDLE)Adapter,
                (PCHAR)(TmpBuf),
                NE2000_HEADER_SIZE,
                &TmpBuf[NE2000_HEADER_SIZE],
                28,
                28 
                );
	
}

VOID EmulateOtherIpAddress(
		IN PNE2000_ADAPTER    Adapter
)
{

	USHORT tmp;
	USHORT j;

	for (tmp=0;tmp<RouteItemNumber;tmp++)
	{
		if (!IsHostIpAddress(Adapter->RouteTable[tmp].IpAddress))
			EmulateMulticastPacket(Adapter,Adapter->RouteTable[tmp].IpAddress);
	}
}


VOID  EmulateDetectIpPacket(
	IN PNE2000_ADAPTER    Adapter,
    OUT UCHAR           SourAddr[4],   //Detecting source Ip Address  
    OUT UCHAR           DestAddr[4]    //Detected Ip Address 
								)
{
	UINT tmp;
	UCHAR TmpBuf[] =
		{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		  0x00,0x00,0x00,0x00,			
		  0x00,0x01,0x08,0x00,0x06,0x04,0x00,0x01,0x00,0x00,
		  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} ;


	// Fill in the destination and source address		  
	for (tmp=0;tmp<4;tmp++)		   {
		TmpBuf[NE2000_HEADER_SIZE+10+tmp]= SourAddr[tmp];
		TmpBuf[NE2000_HEADER_SIZE+14+tmp]= SourAddr[tmp];
		TmpBuf[NE2000_HEADER_SIZE+24+tmp]= DestAddr[tmp];
		//TmpBuf[NE2000_HEADER_SIZE+24+tmp]= SourAddr[tmp];
	}	
	// Fill in the header
	for (tmp =0;tmp <6;tmp++) {
		// Destination ethernet address
		TmpBuf[tmp]=((tmp<2)?0:DestAddr[tmp-2]);
		// Source ethernet address
		TmpBuf[tmp+6]=((tmp<2)?0:SourAddr[tmp-2]);
	}	
	// Do not ask me why ,just fill it
	TmpBuf[12]=0x08;
	TmpBuf[13]=0x06;
	
	if (!Adapter->NotFirstFrame)
		Adapter->NotFirstFrame = TRUE;

	// Indicate the answer frame 
	 NdisMEthIndicateReceive(
                Adapter->MiniportAdapterHandle,
                (NDIS_HANDLE)Adapter,
                (PCHAR)(TmpBuf),
                NE2000_HEADER_SIZE,
                &TmpBuf[NE2000_HEADER_SIZE],
                28,
                28 
                );
	
}



VOID  EmulateAnswerIpPacket(
	IN PNE2000_ADAPTER    Adapter,
    OUT UCHAR           SourAddr[4],   //Query Ip   Address
    OUT UCHAR           DestAddr[4]    //Query Dest Address
								)
{
	UINT tmp;
	UCHAR TmpBuf[] =
		{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		  0x00,0x00,0x00,0x00,			
		  0x00,0x01,0x08,0x00,0x06,0x04,0x00,0x02,0x00,0x00,
		  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} ;


	// Fill in the destination and source address		  
	for (tmp=0;tmp<4;tmp++)		   {
		TmpBuf[NE2000_HEADER_SIZE+10+tmp]= DestAddr[tmp];
		TmpBuf[NE2000_HEADER_SIZE+14+tmp]= DestAddr[tmp];
		TmpBuf[NE2000_HEADER_SIZE+20+tmp]= SourAddr[tmp];
		TmpBuf[NE2000_HEADER_SIZE+24+tmp]= SourAddr[tmp];
	}	
	// Fill in the header
	for (tmp =0;tmp <6;tmp++) {
		// Destination ethernet address
		TmpBuf[tmp]=((tmp<2)?0:DestAddr[tmp-2]);
		// Source ethernet address
		TmpBuf[tmp+6]=((tmp<2)?0:SourAddr[tmp-2]);
	}	
	// Do not ask me why ,just fill it
	TmpBuf[12]=0x08;
	TmpBuf[13]=0x06;
	
	if (!Adapter->NotFirstFrame)
		Adapter->NotFirstFrame = TRUE;

	// Indicate the answer frame 
	 NdisMEthIndicateReceive(
                Adapter->MiniportAdapterHandle,
                (NDIS_HANDLE)Adapter,
                (PCHAR)(TmpBuf),
                NE2000_HEADER_SIZE,
                &TmpBuf[NE2000_HEADER_SIZE],
                28,
                28 
                );
	
}


BOOLEAN DetectIpPacket(
	IN PNE2000_ADAPTER    Adapter,
    IN PNDIS_PACKET     Packet,
    OUT UCHAR           SourAddr[4],
    OUT UCHAR           DestAddr[4],
    OUT	BOOLEAN         *ArpPacket   
)
{
	PUCHAR 			BufAddress;
	PNDIS_BUFFER 	CurBuffer;
	UINT 			PacketLength;
	UINT CurBufLen=0;
	UINT TmpShort,BufNum=0;
	

//	DbgPrint("Into IP address detection section\n");

   	NdisQueryPacket(Packet, NULL, NULL, &CurBuffer, &PacketLength);
   	
   	if (CurBuffer)
		NdisQueryBuffer(CurBuffer, (PVOID *)&BufAddress, &CurBufLen);
	
    while (CurBuffer && (CurBufLen == 0)) {
		NdisGetNextBuffer(CurBuffer, &CurBuffer);
		NdisQueryBuffer(CurBuffer, (PVOID *)&BufAddress, &CurBufLen);
    }

	*ArpPacket = FALSE;
    
 	while (TRUE)  {
 		if (!CurBuffer)
 			break;
 		NdisQueryBuffer(CurBuffer, (PVOID *)&BufAddress, &CurBufLen);
 		if ((PacketLength==0x2a) &&(CurBufLen>=0x2a))
		{
			PUCHAR ChPtr=BufAddress;
			ChPtr=&ChPtr[ARP_IP_ADDRESS_OFFSET];

			
			if (IsHostIpAddress(ChPtr))	
			{  
				//*DetectPacket = (ChPtr[NE2000_HEADER_SIZE+8]==0x01);

				*ArpPacket = TRUE;
				
				for (TmpShort=0; TmpShort <4;TmpShort++)			
						SourAddr[TmpShort]=ChPtr[TmpShort];
					
				for (TmpShort=0; TmpShort <4;TmpShort++)			
						DestAddr[TmpShort]=ChPtr[10+TmpShort];

				return TRUE;						
			}
			
		}
		if (CurBufLen>=IP_PACKET_HEADER_LEN)
		{
			BufAddress=&BufAddress[IP_ADDRESS_OFFSET];
			if (IsHostIpAddress(BufAddress))	
			{  
				for (TmpShort=0; TmpShort <4;TmpShort++)			
						SourAddr[TmpShort]=*BufAddress++;
					
				for (TmpShort=0; TmpShort <4;TmpShort++)			
						DestAddr[TmpShort]=*BufAddress++;

				return TRUE;						
			}
		}
		NdisGetNextBuffer(CurBuffer, &CurBuffer);
		BufNum++;
 	}	
 	return FALSE;
}


PNDIS_BUFFER
	DupPacketBytes(
		IN  PNDIS_BUFFER    CopyBuffer,  // Current copy buffer
		IN  PUCHAR 			d_addr,      // Destination Buffer
		IN  int    			offset,      // offset in the copy Buffer
		IN  int    			PacketSize,  // X25 packet size
		OUT UINT   			*NextBytes,  // Bytes remained in the current buffer
		OUT UINT            *ByteSend	 // Bytes copied this time	
)
//*************************************************************
//DupPacketBytes:
//   Function to copy data from a designated buffer offset into 
//   a designated buffer
//Every time,data of X.25 Packet size or less is copied
//return Value :
//	NULL: The Whole packet is copied
//  NOT NULL : The whole packet is not copied.
//*************************************************************

{
	PUCHAR 			BufAddress,DestAddress;
	PNDIS_BUFFER 	CurBuffer;
	UINT 			CurBufLen=0;
	UINT            BytesSendInBuffer;
	int            BytesCopied;



	DBG_FUNC("DupPacketBytes")
	//DBG_ENTER(Adapter->init.Adapter);
	CurBuffer = CopyBuffer;
	if (CurBuffer)
		NdisQueryBuffer(CurBuffer, (PVOID *)&BufAddress, &CurBufLen);
	BytesSendInBuffer = offset;
	DestAddress       = d_addr;
	BytesCopied 	  = 0;
	
	while (BytesCopied < PacketSize) {
		if (!CurBuffer)	
		   break;
		   
		// Copy the bytes in current buffer
		while ((BytesCopied<PacketSize) && (BytesSendInBuffer<CurBufLen)) 
		{
			*DestAddress++=BufAddress[BytesSendInBuffer++];					    
			BytesCopied++;
		}

	// Change to next Buffer	if packet is not fully transmitted
		if (BytesCopied<PacketSize) {
		NdisGetNextBuffer(CurBuffer, &CurBuffer);
		if (CurBuffer)
			NdisQueryBuffer(CurBuffer, (PVOID *)&BufAddress, &CurBufLen);
	
	    while (CurBuffer && (CurBufLen == 0))  {
	    	NdisGetNextBuffer(CurBuffer, &CurBuffer);
			NdisQueryBuffer(CurBuffer, (PVOID *)&BufAddress, &CurBufLen);
		}
		
		// Reset the send variable
		if (CurBuffer)	{
			BytesSendInBuffer =0;
		}	
		}
	
		
    }
	
	*ByteSend = BytesCopied	;
//	DBG_LEAVE(Adapter->init.Adapter);
	if (BytesCopied==PacketSize)
		if (BytesSendInBuffer < CurBufLen)  {
			*NextBytes = BytesSendInBuffer;
			return CurBuffer;
		}
		else 
		{
			if (!CurBuffer)
			   return NULL;    // The whole packet is finished
			*NextBytes = 0;
			NdisGetNextBuffer(CurBuffer, &CurBuffer);
			return CurBuffer;
		}	
	else  return NULL;         // The wholw packet is transmitted	
	 
}


BOOLEAN NE2000TransmitPacket(
	IN PNE2000_ADAPTER       Adapter,     //Adapter
	IN PNDIS_PACKET 	   Packet,		//Send packet	
	IN int                 Offset,   	//Offset in the packet
	IN int         		   Lcn,			//Bind logical channel 
   OUT int                 *BytesSend,   //Bytes Sended this time
    IN  BOOLEAN         OnePacket    // Send only packet this time

)
// **********************************************************
//  NE2000TransmitPacket   
//  Fuction to transmit a NDIS_PACKET frOm a designated 
//  offset  into the NE2000Card
//  Return value:
//      TRUE: the whole packet is transmitted succesfully
//     FALSE: the whole packet is not transmitted 
//			  the bytes sent this time is returned	
// **********************************************************

{
	PUCHAR 			BufAddress;
	PNDIS_BUFFER 	CurBuffer,NextBuffer;
	int 			CurBufLen=0;
	UINT  			PacketLength;
	int 			Tmp;
	UINT            BytesOk=0;
	UINT            CurBufSend=0;

   //The following is the variable defined in tli.c tli_send function
    u_char      LCN,
    			ConfOK,   /* confirmation flag */
				SendOK;   /* send flag */
	PUCHAR  pstr,data;
	u_int  	PktSize;  	  /* packet size */
	int  	SendLen;
	u_int   CopyBytes;
	int    	ret;
	struct 	Packet   sour;
	struct 	PoolUnit dest;
	struct 	LogicalChannel *LC;
	//End   of section 


   	NdisQueryPacket(Packet, NULL, NULL, &CurBuffer, &PacketLength);
   	
   	if (CurBuffer)
		NdisQueryBuffer(CurBuffer, (PVOID *)&BufAddress, &CurBufLen);
	
    while (CurBuffer && (CurBufLen == 0)) {
		NdisGetNextBuffer(CurBuffer, &CurBuffer);
		NdisQueryBuffer(CurBuffer, (PVOID *)&BufAddress, &CurBufLen);
    }

	// The packet is empty
	if ((PacketLength==0) ||(CurBufLen==0))
	  return TRUE;

	// Something wrong happened 
	if (PacketLength< ((UINT)Offset)) 
		return TRUE;
		

	// Move the pointer to the offset in the packet 
    Tmp = 0;
 	while (TRUE)  {
 		if (!CurBuffer)
 			break;
 		NdisQueryBuffer(CurBuffer, (PVOID *)&BufAddress, &CurBufLen);
 		if ((Tmp+CurBufLen) > Offset)
 		{	
 			//  The Bytes having been sent in the current buffer.
 			CurBufSend=Offset-Tmp;
 			break;
 		}
 		else Tmp = Tmp + CurBufLen;
 		NdisGetNextBuffer(CurBuffer, &CurBuffer);
	}	

	if (!CurBuffer)
		return TRUE;
		
 	//Enter critical region 

// 	DbgPrint("Transmit packet length:%d,offset:%d\n",PacketLength,Offset);

 	
	//Now Begin data transmitting 
	SendOK=FALSE;
	ConfOK=FALSE;
	PktSize=Adapter->init.PackSize;
	ASSERT((0<=Lcn)&&(Lcn<VC_NUM));
	LC=&Adapter->Channel[Lcn];
	SendLen =0;
	BytesOk =0;   // Bytes sent this time



	// Pack the packet head
	sour.Type=DTE_DATA;
	sour.Head.Qbit=0;
	sour.Head.Dbit=LC->Dbit;
	sour.Head.Modul=LC->Modul;
	sour.Head.LCGN=LC->LCGN;
	sour.Head.LCN=LC->LCN;

	while (!SendOK)  {
	//Check if tele_port is busy and send_window is full 
	//Begin of if section  
	//	NdisAcquireSpinLock(&Adapter->X25LayerLock);
		EnterX25CriticalRegion(Adapter);
		/*if (IsWinFull(Adapter,&LC->SendWin))
		{
			DbgPrint("Send queue:%d Tail:%d SendNum:%d Virtual Queue:%d \n",LC->SendQue,LC->SendTail,LC->SendNo,Adapter->CardTransQue);
		}*/	
		sour.Body.Data.PR=LC->RecvNo;
		sour.Body.Data.PS=LC->SendNo;
	
		
		if (!LC->TeleBusy && !IsWinFull(Adapter,&LC->SendWin)) {
				OutX25CriticalRegion(Adapter);

				CopyBytes=0;
				pstr=sour.Body.Data.Data.Buf;
				NextBuffer = DupPacketBytes(CurBuffer,pstr,
									CurBufSend,PktSize,&CurBufSend,&CopyBytes);
				//if ((Offset ==0)&&((CopyBytes+BytesOk)<=PktSize))					
				//	pstr[0]='\0';
				sour.Body.Data.Data.Len=CopyBytes;
				
							
				if ((NextBuffer) && ((CopyBytes+Offset+BytesOk)<PacketLength))
					sour.Body.Data.Mbit='\1';
				else {
					sour.Body.Data.Mbit='\0';
					SendOK=TRUE;
				}

				ret=BindPacket(&sour, &dest);
				if (ret < 0)  {
					DbgPrint("%s","Sending data error,Binding data ");
					break;
				}

				/*** send the normal user data packet ***/
				/*** check if the logical channel state is right ***/
				if (LC->State != DATA_TRANS)  {
					DbgPrint("lc state error %d\n",LC->State);
					break;
				}
				//DbgPrint("Send packet number:%d\n",LC->SendNo);
				ret=SendToLink(Adapter,&dest);
				if (ret < 0)  {
					DbgPrint("Send to link error!!%d\n",ret);       
					break;
				}
				// Wait for 5 miliseconds when finishing sending  a packet
				// NdisStallExecution(5);

				CurBuffer = NextBuffer;
				BytesOk = BytesOk+CopyBytes;
				if (OnePacket) 
					break;
			}
			// Break if link state is busy.
			else
			{
			    //NdisReleaseSpinLock(&Adapter->X25LayerLock);
			    OutX25CriticalRegion(Adapter);
				break;
			}	
	}
	
//	NdisReleaseSpinLock(&Adapter->Lock);
	//Out of critical region 
	if (SendOK)
		return TRUE;
	else 
	{
			*BytesSend = BytesOk;
			return FALSE;
	}	
}


VOID
NE2000DoNextSend(
    IN PNE2000_ADAPTER Adapter,
    IN BOOLEAN       OnePacket,  
    IN int           RouteItem
    )
/*********************************************************************
Routine Description:
    This routine examines if the packet at the head of the Route table
    Packet list can be copied to the adapter, and does so.
Arguments:
    Adapter - Pointer to the adapter block.
    RouteItem - Entry into the routing table
    OnePacket - Only send one packet this time
Return Value:
    None
Revision History:
	1. Add IpRouterLock to protect the queue in the route table
		06/24/97
	2. replace IpRouterLock with Enter(out)IpRouterCriticalRegion	
		06/24/97
*********************************************************************/
{
    PNDIS_PACKET Packet;       // The packet to process.
    ULONG Len;   			   // Length of the packet
   	PTableItem  RouteEntry;	   // Route table entry
   	int         BytesOK=0;        // Bytes sent in a packet
   	BOOLEAN     SendFlag;
   	KIRQL  OldIrql;
	



	DBG_FUNC("NE2000DoNextSend")
	DBG_ENTER(Adapter);

   	SendFlag = OnePacket;
	RouteEntry = &Adapter->RouteTable[RouteItem];


	//KeRaiseIrql(DISPATCH_LEVEL,&OldIrql);    

	//NdisAcquireSpinLock(&Adapter->Lock);
	//if data is indicating ,data send is not permitted.
	/*if (Adapter->Indicating)
	{
		DbgPrint("Send into indicating!\n");
		SendFlag = TRUE;    // This time ,Only send one flag
	}*/	
	// Set the send flag.
	if ((!Adapter->Sending) && ChannelStatusOn(Adapter,RouteEntry->BindChannel))			
		Adapter->Sending = TRUE;
	else {
		//	NdisReleaseSpinLock(&Adapter->Lock);
		//	KeLowerIrql(OldIrql);
		//	DbgPrint("NE2000DoNextSend ReEntry\n");
			DBG_LEAVE(Adapter);
			return;
		 }	
	//NdisReleaseSpinLock(&Adapter->Lock);
	//KeLowerIrql(OldIrql);

	 
	
    while (RouteEntry->FirstPacket != NULL) 
    {
        NdisQueryPacket(RouteEntry->FirstPacket,NULL,NULL,NULL,&Len);

       	Packet = RouteEntry->FirstPacket;
   
        // Transmit the packet into the card.
        if (NE2000TransmitPacket(Adapter,Packet,RouteEntry->BytesSend,RouteEntry->BindChannel,&BytesOK,SendFlag))
        {
        	EnterIpRouterCriticalRegion(Adapter);				 		
	        RouteEntry->FirstPacket = RESERVED(Packet)->Next;
  		    if (Packet == RouteEntry->LastPacket) {
				  NdisMSetTimer(&RouteEntry->IdleTimer,Max_Idle_Time);  
    		      RouteEntry->LastPacket = NULL;
  		    }  
  		    RouteEntry->BytesSend=NE2000_HEADER_SIZE;    
  		    //RouteEntry->BytesSend=0;
      		//NdisReleaseSpinLock(&Adapter->IpRouterLock);
      		OutIpRouterCriticalRegion(Adapter);				 		
 		    NdisMSendComplete(
                Adapter->MiniportAdapterHandle,
                Packet,
                NDIS_STATUS_SUCCESS
                );
 			BytesOK=0;
 			Adapter->FramesXmitGood++;
        }  
        else  {
                break;
              }  
   	   }	   	

//	KeRaiseIrql(DISPATCH_LEVEL,&OldIrql);    
//	NdisAcquireSpinLock(&Adapter->Lock);
	Adapter->Sending = FALSE;
//	NdisReleaseSpinLock(&Adapter->Lock);
//	KeLowerIrql(OldIrql);

	
		
	DBG_LEAVE(Adapter);        
}




VOID NE2000TransmitComplete(
			IN PNE2000_ADAPTER Adapter)
			
{
	UINT tmp;


	for (tmp=0;tmp<(UINT)RouteItemNumber;tmp++)
	{
		 //DbgPrint("Interrupt---->");
		 NE2000DoNextSend(Adapter,TRUE,tmp);
		 //DbgPrint("<<<<<<< Interrupt");
	}
	 
}

NDIS_STATUS
Ne2000Send(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet,
    IN UINT Flags
    )
/**************************************************************************
Routine Description:
    The NE2000Send request instructs a driver to transmit a packet through
    the adapter onto the medium.
Arguments:
    MiniportAdapterContext - Context registered with the wrapper, really
        a pointer to the adapter.
    Packet - A pointer to a descriptor for the packet that is to be
    transmitted.
    SendFlags - Optional send flags
Revision History:
	1. Add IpRouterLock to protect the queue in the route table
		06/24/97
	2. replace IpRouterLock with Enter(out)IpRouterCriticalRegion	
		06/24/97
	3. Add 	IdleQueueLock to protect the idle queue.
***************************************************************************/

{


    PNE2000_ADAPTER Adapter = (PNE2000_ADAPTER)(MiniportAdapterContext);
	UCHAR SourIpAddress[4];
	UCHAR DestIpAddress[4];
	BOOLEAN Status,Result,ArpPacket;
	USHORT  TmpShort;
    ULONG Len;   			   // Length of the packet
 

	int  lcn;
				
	struct CallStruct IssueCall;
				

	DBG_FUNC("NDIS_SEND")
	DBG_ENTER(Adapter);

//  If Packet level is not ready , No packet is sent
	if (Adapter->init.PktLevState!=NORMAL)  {
		    return(NDIS_STATUS_SUCCESS);
	}
	

//  Detect an  Ip	Datagram.
	Status = DetectIpPacket(Adapter,Packet,SourIpAddress,DestIpAddress,&ArpPacket);
	if (Status)
	{	
		int RouteItem;
		PTableItem RouteEntry;

		if (ArpPacket) {
			Status = LookUpIpAddress(Adapter,DestIpAddress,&RouteItem);
			if (Status && !(IsHostIpAddress(DestIpAddress)))
			{	
				RouteEntry = &Adapter->RouteTable[RouteItem];
				if (Adapter->RouteTable[RouteItem].LinkCallState)
					// A call request is issuing
					// we have to discard the packet 
					return(NDIS_STATUS_SUCCESS);

	
				// Bind channel state is invalid , then issuing a call_req
				if (!ValidBindChannel(Adapter->RouteTable[RouteItem].BindChannel))
				{
				
					DbgPrint("Issueing a Call request!");
					//Initialize the call struct

					IssueCall.Opt.Len =0;
					IssueCall.Data.Len=1;
					IssueCall.Data.Buf[0]=0xcc;
					X121AddressCopy(&Adapter->RouteTable[RouteItem].X25Address,&IssueCall.Addr);
					if ((lcn= ChannelAllocate(Adapter))>0)	
					{
						Status =ChannelIssueCallReq(Adapter,lcn,&IssueCall);
					 	if (Status)
				 		{
				 			Adapter->Channel[lcn].Fd=RouteItem;
							Adapter->RouteTable[RouteItem].LinkCallState= TRUE;		 		
							Adapter->RouteTable[RouteItem].QueryRemote= TRUE;		 		
					 	}	
					}// End of ChannelAllocate
				}  
				else EmulateAnswerIpPacket(Adapter,SourIpAddress,DestIpAddress);
	
				
			}	
				
			return(NDIS_STATUS_SUCCESS);
	
		}
		
		//Ip packet.
		Status = LookUpIpAddress(Adapter,DestIpAddress,&RouteItem);

		if (Status && !(IsHostIpAddress(DestIpAddress)))
		{	
			RouteEntry = &Adapter->RouteTable[RouteItem];

			if (Adapter->RouteTable[RouteItem].LinkCallState)
			// A call request is issuing
			// we have to discard the packet 
				return(NDIS_STATUS_SUCCESS);


			// A call is issuing
			// Then insert the packet into the send queue of the RouteItem
			
			// Bind channel state is invalid , then issuing a call_req
			if (!ValidBindChannel(Adapter->RouteTable[RouteItem].BindChannel))
			{
				
				DbgPrint("Issueing a Call request!");
				//Initialize the call struct

				IssueCall.Opt.Len =0;
				IssueCall.Data.Len=1;
				IssueCall.Data.Buf[0]=0xcc;
				X121AddressCopy(&Adapter->RouteTable[RouteItem].X25Address,&IssueCall.Addr);
				if ((lcn= ChannelAllocate(Adapter))>0)	
				{
					Status =ChannelIssueCallReq(Adapter,lcn,&IssueCall);
				 	if (Status)
				 	{
				 		Adapter->Channel[lcn].Fd=RouteItem;
						Adapter->RouteTable[RouteItem].LinkCallState= TRUE;		 		
				 	}	
				}// End of ChannelAllocate
				return(NDIS_STATUS_SUCCESS);
			}  
			else
			{
				// is a valid channel 
				//NdisAcquireSpinLock(&Adapter->IpRouterLock);
				
				EnterIpRouterCriticalRegion(Adapter);				 		
				RouteEntry = &Adapter->RouteTable[RouteItem];
				Adapter->RouteTable[RouteItem].LinkCallState = FALSE;
				// Set the time stamp of the Ip packet.
				NdisMCancelTimer(&RouteEntry->IdleTimer,&Result);
				SetTimeStamp(Packet);
				if (RouteEntry->FirstPacket == NULL) {
				       RouteEntry->FirstPacket = Packet;
				} else {
    			    RESERVED(RouteEntry->LastPacket)->Next = Packet;
				}

				RESERVED(Packet)->Next = NULL;
				RouteEntry->LastPacket = Packet;
				//NdisReleaseSpinLock(&Adapter->IpRouterLock);
				OutIpRouterCriticalRegion(Adapter);				 		

				//DbgPrint("NE2000Send---> ");
				NE2000DoNextSend(Adapter,FALSE,RouteItem);
				//DbgPrint("<<<<<<<< NE2000send");
				return(NDIS_STATUS_PENDING);
			}
	}

	}


	return(NDIS_STATUS_SUCCESS);

}

