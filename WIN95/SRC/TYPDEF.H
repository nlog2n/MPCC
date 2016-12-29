/*TYPDEF.H*/
#ifndef _typdef_h
#define _typdef_h

#include <ndis.h>
#include "const.h"
#include "ne2000sw.h"

struct NetAddr  {
	u_int    Len;
	u_char   Addr[ADDRLEN+1];
};
#define MAX_LENGTH  20
typedef struct tag_MY_ANSISTRING{
     u_int  Length;
     u_char  Buffer[MAX_LENGTH+1];
     }  MY_ANSISTRING,*PMY_ANSISTRING;

#define MAX_ADDR_PAIR_LENGTH  1000
typedef struct tag_ADDR_PAIR_STRING{
     u_int  Length;
     u_char  Buffer[MAX_ADDR_PAIR_LENGTH+1];
     }  ADDR_PAIR_STRING,*PADDR_PAIR_STRING;
     

typedef struct tableItem {
	UCHAR IpAddress[4];			//Ip address of the route item 
	struct NetAddr  X25Address; //X25 address of the route item
	int    	  BindChannel;      //X25 channel ready for data transmitting
	BOOLEAN   QueryRemote;      //boolean to indicate a remote node is being queried
	BOOLEAN   LinkCallState;    //boolean variable to mark whether a call is being established
	PNDIS_PACKET FirstPacket;	//The head of the pending queue
    PNDIS_PACKET LastPacket;    //The tail of the pending queue.
	int       BytesSend;        //The bytes have been sent in the first packet
	int       Priorty;        	//The number to determine the time of Arrived packet.
//	int       IdleTime;         //The retry number of the first queued pacekt.
	NDIS_MINIPORT_TIMER  IdleTimer; //Idle timer to monitor each RouteTable Entry
	PVOID      NE2000Adapter;
}       TableItem,*PTableItem;


struct  pacnictl        {
	int     PktLevState;         /* packet level state */
	u_char  DteAddr[ADDRLEN+1];  /* only one address */
	u_char  DceAddr[ADDRLEN+1];
	struct  NetAddr DteAddress;
	u_char  WinSize;             /* Size of the window */
	u_int   PackSize;            /* Size of the packet */
	u_long  PutSize;             /* The throughput value */
	u_char  Modul;               /* modulo 8 or modulo 128 */
	u_char  NetUsrId[NAMELEN+1]; /* network user id */
	u_short T20,T21,T22,T23,T28; /* Timers */
	u_char  LIC;                 /* one way lowest incoming call */
	u_char  HIC;                 /* one way hignest incoming call */
	u_char  LTC;                 /* two way lowest call */
	u_char  HTC;                 /* two way highest call */
	u_char  LOC;                 /* one way lowest outgoing call */
	u_char  HOC;                 /* one way hignest outgoing call */
};



struct UserData  {
	u_int     Len;
	u_char    Buf[DATALEN+1];
};


struct IntData  {
       u_int   Len;
       u_char  Buf[INTLEN+1];
};


struct Option  {
       u_int     Len;
       u_char    Buf[OPTLEN+1];
};


struct Regist  {
       u_int    Len;
       u_char   Buf[REGLEN+1];
};


struct BindStruct  {
      u_char          Type;
      u_char          VcType;
      struct NetAddr  BindAddr;
};


struct CallStruct  {
	struct NetAddr  Addr;
	struct Option   Opt;
	struct UserData Data;
};


struct  DisconStruct  {
	struct UserData Data;
	struct Option   Opt;
	  u_char        Cause;
};


typedef u_char PKT_TYP[DATALEN+HEADLEN+1];


struct PoolUnit  {
       u_char   Type;     /* Data type of the Unit */
       u_char   LCN;      /* Logical channel number */
       u_char   Mbit;     /* More data bit */
       u_int    Len;      /* data length */
       PKT_TYP  Data;     /* Data Buffer */
       int      Next;     /* Index of Next Unit in Queue */
       int      TransmitNext; //Index of mext unit in the transmit queue
};


struct Window  {
       u_char  Low,  /* window lower edge */
	       High, /* window upper edge */
	       Size; /* current window size */
};


struct  LogicalChannel  {
	 u_char   LCGN,       /* Logical Channel Group Number */
		      LCN;        /* Logical Channel Number */
	  int     State;      /* state of LCN */
	 u_char   Cause,      /* reset, restart or clear cause */
			  DiagCode;
	 u_char   Dbit,       /* Delivery confirmation bit */
		      Modul;      /* Modul 8(=0x10) or modulo 128(=0x20) */
	 int      SendQue,    /* Send Queue Head & Tail Index */
		      SendTail,
		      RecvQue,    /* Receive Queue Head & Tail Index */
		      RecvTail;
	 struct Window SendWin;    /* Send Window */
	 u_char   SendNo,     /* Send & Receive Sequence number */
		      RecvNo;
	 char     Expedited,  /* Flag of Transmiting Expedited Data */
		      TeleBusy;   /* Flag of Tele_DTE Busy */
	 int      Fd;         /* FdArray index */
	 int tcp_len;

    // This timer is used to provide timing service to each LCN.

    // NDIS_MINIPORT_TIMER CallTimer;
    
    // Link handle to map the link structure
	 //PHTDSU_LINK         LinkHandle;

};


struct  FdStruct
{
  int LCN  ;
  /*  LCN = -1  : FDUNUSED
		  FDUNOPENED
		  FDCLOSED
      LCN = -2  : FDUNBIND
      LCN = -3  : FDUNCONNETED
      0 < LCN < 256 : LCN  */

     u_char      VcType;      /* Virtul Circuit Type */
  struct NetAddr CallAddr,    /* Calling DTE Address  */
		 CalledAddr;  /* Called DTE Addreess  */
  /*add for PC/TCP compatible*/
  int tcp_call;
};


   /* Packet Header Struct Definition */
struct PktHead   {
       u_char    Qbit;
       u_char    Dbit;
       u_char    Modul;
       u_char    LCGN;
       u_char    LCN;
};


   /* Call set_up Packet Struct */
struct PktCall   {
  struct NetAddr   CallAddr,
		   CalledAddr;
  struct Option    Opt;
  struct UserData  Data;
};


  /* User Data Packet Struct */
struct PktData   {
       u_char     PR,
		  PS;
       u_char     Mbit;
  struct UserData Data;
};


  /* Flow control Packet Struct */
struct PktFlow   {
       u_char   PR;
};

  /* Interrupt Packet Struct   */
struct PktInt    {
   struct IntData  Data;
};


  /* Reset Packet Struct*/
struct PktReset  {
       u_char  Cause;
       u_char  DiagCode;
};


  /* Restart Packet Struct */
struct PktRestart  {
       u_char  Cause;
       u_char  DiagCode;
};


  /* Clear Packet Struct*/
struct PktClear    {
       u_char     Cause;
       u_char     DiagCode;
  struct NetAddr  CallAddr,
		  CalledAddr;
  struct Option   Opt;
  struct UserData Data;

};


  /* Diagnostic Packet Struct */
struct PktDiagnose  {
       u_char   DiagCode;
       u_char   Explan[HEADLEN+1];
};


  /* Register Packet Struct */
struct PktRegist   {
	  u_char      Cause;
	  u_char      DiagCode;
       struct NetAddr DteAddr,
		      DceAddr;
       struct Regist  Reg;
};


  /*  Packet Struct */
struct Packet     {
	   u_char        Type;        /*  Packet Type */
       struct   PktHead  Head;        /*  Packet Head */
  union   {
    struct   PktCall     Call;
    struct   PktData     Data;
    struct   PktFlow     Flow;
    struct   PktInt      Int;
    struct   PktReset    Reset;
    struct   PktRestart  Restart;
    struct   PktClear    Clear;
    struct   PktRegist   Regist;
    struct   PktDiagnose Diagnose;
  }Body;                              /*  Packet Body */
};
#endif
