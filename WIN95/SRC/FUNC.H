/*FUNC.H*/

     /*************************************************************

		    function prototypes

     **************************************************************/

#ifndef _func_h
#define  _func_h
#include "typdef.h"

#include "ne2000sw.h"

/*** Copy & compare group ***/
VOID EnterX25CriticalRegion(
		IN PNE2000_ADAPTER   Adapter
							);

VOID OutX25CriticalRegion(
		IN PNE2000_ADAPTER   Adapter
							);

VOID EnterCardCriticalRegion(
		IN PNE2000_ADAPTER   Adapter
							);

VOID OutCardCriticalRegion(
		IN PNE2000_ADAPTER   Adapter
							);

VOID EnterIpRouterCriticalRegion(
		IN PNE2000_ADAPTER   Adapter
							);

VOID OutIpRouterCriticalRegion(
		IN PNE2000_ADAPTER   Adapter
							);
							
							

							
VOID TransmitQueue(
			  IN PNE2000_ADAPTER   Adapter
);


void enable();
void disable();
void d_prn(char * str);
void StrCopy(u_char  *dest, u_char  *sour, u_int length);
void AddrCopy(struct NetAddr  *dest, struct NetAddr  *sour);
int AddrComp(struct NetAddr  *dest, struct NetAddr  *sour);
int  StrComp(u_char  *str1, u_char *str2, u_int length);

/*** check & search group ***/
int  SearchFd(void);
int  CheckPktHead(struct LogicalChannel *lc, struct PktHead *head);

/*** window operate group ***/
u_char GetNextWinNo(u_char modul, u_char num);
u_char GetWinSize(IN PNE2000_ADAPTER   Adapter,struct Window *win);
int  IsWinFull(IN PNE2000_ADAPTER   Adapter,struct Window *win);

/*** send procedure group ***/
int  BindPacket(struct Packet *sour, struct PoolUnit *dest);
int  SendToLink(IN PNE2000_ADAPTER   Adapter,
				IN struct PoolUnit *unit);

/*** send packet group ***/
int  SendFlowCtlPkt(IN PNE2000_ADAPTER   Adapter,
					IN struct LogicalChannel *lc, u_char type);
int  SendIntConfPkt(u_char lcn);
int  SendResetPkt(IN PNE2000_ADAPTER   Adapter, IN u_char lcn,
				  IN u_char type, IN u_char cause,IN  u_char code);
int  SendRestartPkt(IN PNE2000_ADAPTER   Adapter,
					IN u_char type,IN u_char cause,IN u_char code);

/*** receive procedure group ***/
int  RecvPacket(
		IN PNE2000_ADAPTER   Adapter,
		struct LogicalChannel *lc, u_char *buf,
		u_int length, u_char type);
int  AssemblePacket(
			 IN PNE2000_ADAPTER   Adapter,
			 struct LogicalChannel *lc, u_char  *buf,
			 u_int length, char flags);

/*** channel operate group ***/
void InitChannel(u_char lcn);

/*** group ready for Ling G. Sun ***/
int  GetPacket(u_char lcn, char type, struct Packet *pkt);


/* group recoded by Song. */
void optcpy(struct Option  *desopt,struct Option  *souopt);
void datacpy(struct UserData  *desud,struct UserData  *souud);

/* This function is recoded by Song on Oct. 31, 1995.
	Below is the prototype of Sun's
int file_init( FILE *fp,  char typeflag,  int  low,
			 int  high,  int  errstate,
			 int  *intp, u_char *strp,
			 u_char *charp);  */



void addrcpystb(struct NetAddr  *desaddr,struct NetAddr  *souaddr);
void addrcpybts(struct NetAddr  *desaddr,struct NetAddr  *souaddr);
int  AddrComp(struct NetAddr  *desaddr,struct NetAddr  *souaddr);

void initfdarray(u_char i);

int sendrestart(int fd);
int ReSend(IN PNE2000_ADAPTER   Adapter,
		   IN struct LogicalChannel *LC);

void RestartAll(void);
/* add for PC/TCP*/
int tcp_recv(struct LogicalChannel *LC,int cs,int ip);
int para_init(void);
#endif



