#ifndef _channel_
#define _channel_

#include "typdef.h"

VOID
ChannelInitialize(IN PNE2000_ADAPTER        Adapter,
				  IN int    lcn );

VOID X121AddressCopy(
				IN struct NetAddr *sour,
				IN struct NetAddr *dest
);

BOOLEAN IsHostIpAddress(IN PUCHAR BufAddress);

int IsHostX25Address(
				IN struct NetAddr X25Addr);


BOOLEAN LookUpIpAddress(
		 IN PNE2000_ADAPTER    Adapter,
		 IN  UCHAR     IpAddress[4],
		 OUT int     *RouteItemNum
);	

BOOLEAN UpLoadCallReq(
		IN PNE2000_ADAPTER    Adapter,
		IN  struct NetAddr *s_addr,
		IN  int    lcn
);


VOID PacketLevelInitialize(IN PNE2000_ADAPTER   Adapter);

VOID LapbReady(	IN PNE2000_ADAPTER    Adapter);

int  ChannelAllocate(IN PNE2000_ADAPTER   Adapter );

BOOLEAN ChannelStatusOn(
				    IN PNE2000_ADAPTER  Adapter,
					IN  int    lcn);			


BOOLEAN ChannelIssueCallReq(
					 IN PNE2000_ADAPTER        Adapter,
					 IN int                lcn,
					 IN struct CallStruct    *SendCall);
BOOLEAN ChannelIssueCallConf(
					 IN PNE2000_ADAPTER        Adapter,
					 IN UCHAR                lcn,
					 IN struct CallStruct    *SendCall);
					 
int 	ChannelIssueClearReq(
					IN PNE2000_ADAPTER        Adapter,
					IN struct LogicalChannel *LC
					);

int ChannelIssueClearConf(
					IN PNE2000_ADAPTER        Adapter,
					IN struct LogicalChannel *lc
					);

VOID RestartPacketLevel(
	 IN PNE2000_ADAPTER        Adapter
);
					
VOID 	UpLoadCallConf(
		IN PNE2000_ADAPTER    Adapter,
		IN struct LogicalChannel *LC);
		
					 
	
VOID UpLoadClearReq(
	IN PNE2000_ADAPTER    Adapter,
	IN struct LogicalChannel *LC		
);

VOID UpLoadCallConf(
		IN PNE2000_ADAPTER    Adapter,
		IN struct LogicalChannel *LC);

VOID UpLoadResetReq(
		IN PNE2000_ADAPTER    Adapter,
		IN struct LogicalChannel *LC);

VOID X25ErrorHandler(
	IN PNE2000_ADAPTER       		Adapter,
	IN UCHAR 			  		ErrorCode,
	IN struct LogicalChannel    *LC
);

BOOLEAN NoMoreDataToSend(
	IN PNE2000_ADAPTER    Adapter
);

#endif

