

/*VARDEF.h*/
#ifndef _extervar_h
#define  _extervar_h

#include  "ne2000sw.h"

extern struct NetAddr    X121_Address;

extern UCHAR HostIpAddress[4];

//extern struct pacnictl   init;

//extern TableItem RouteTable[Max_Ip_Num];

extern struct FdStruct    FdArray[VC_NUM];     /* ? */

//extern struct LogicalChannel    Channel[VC_NUM];

/* Song: logical channels information */

//extern struct PoolUnit   Pool[POOLLEN];
/* Song: System buffer pools */

//extern int   SysFreeQue;
     /* Song: System free queue head index */

/* Song: 5 system timers */
extern u_short  T20,
	 T21,
	 T22,
	 T23,
	 T28;
/* Song: Timer flag */
extern u_char   TimeOut;

/* Song: two far_interrupt_function pointers */
//  The following defination is omitted by Guo
//void far interrupt (*RecvAddr) ();
//void far interrupt (*TimeAddr) ();

/* Song: variables used for timer tick */
extern u_char  ts, tss;

/*chw add*/
/*paket driver interrupt number*/
extern u_char PKT_INT;
/*packet level reciver interrupt level*/
extern u_char RCV_INT;
/*hdlc driver interrupt number*/
extern u_char HDLC_INT;
/*pctcp driver interrupt number*/
extern u_char TCP_INT;
extern int TCP_OK;
extern int para_c;
extern char * para_v[8];
extern char para_buf[80];
extern int r_flag;
extern int t_flag;
#endif
