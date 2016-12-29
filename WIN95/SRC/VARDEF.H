/*VARDEF.h*/
#ifndef _vardef_h
#define  _vardef_h

#include  "ne2000sw.h"

//struct pacnictl   Adapter->init;

//TableItem RouteTable[Max_Ip_Num];

struct FdStruct    FdArray[VC_NUM];     /* ? */

//struct LogicalChannel    Channel[VC_NUM];
/* Song: logical channels information */

//struct PoolUnit   Pool[POOLLEN];
/* Song: System buffer pools */

//int   SysFreeQue;
     /* Song: System free queue head index */

/* Song: 5 system timers */
u_short  T20,
	 T21,
	 T22,
	 T23,
	 T28;
/* Song: Timer flag */
u_char   TimeOut;

/* Song: two far_interrupt_function pointers */
//  The following defination is omitted by Guo
//void far interrupt (*RecvAddr) ();
//void far interrupt (*TimeAddr) ();

/* Song: variables used for timer tick */
u_char  ts=0, tss=0;

/*chw add*/
/*paket driver interrupt number*/
u_char PKT_INT;
/*packet level reciver interrupt level*/
u_char RCV_INT;
/*hdlc driver interrupt number*/
u_char HDLC_INT;
/*pctcp driver interrupt number*/
u_char TCP_INT;
int TCP_OK;
int para_c;
char * para_v[8];
char para_buf[80];
extern int r_flag;
extern int t_flag;
#endif
