
//#ifndef NULL
#ifndef NULL
#define NULL   0
#endif 
/***** define abbreviations:*******/
#ifndef u_char
#define u_char  unsigned char
#endif

#ifndef u_int
#define u_int   unsigned int
#endif

#ifndef u_short
#define u_short unsigned short
#endif 

#ifndef u_long
#define u_long  unsigned long
#endif 

#ifndef _const_h
#define _const_h

#define ValidBindChannel(channel)        (channel!=-1)
#define Max_Ip_Num                               16	  //Support 16 Ip Address
#define Idle_Timer_Interval						 10000 // 10s for idle timer
#define Max_Idle_Time                            15000 // 30s for idle time
#define EqualIpAddress(addr1,addr2)  ((addr1[0]==addr2[0])&&(addr1[1]==addr2[1])&&(addr1[2]==addr2[2])&&(addr1[3]==addr2[3]))
#define Monitor_Interval						 600  //offline monitor interva 200ms
#define MAX_ENDURE_TIME    			             5

#define DISPATCH_LEVEL   2


/*** define macro_identifiers:
****/
#define TRUE         1
#define FALSE        0

#define PVC          0
#define SVC          1

#define ADDRLEN      15
#define INTLEN       32
#define OPTLEN       109
#define REGLEN       109
#define DATALEN      256
#define HEADLEN      3
#define NAMELEN      20

#define POOLLEN      90
#define VC_NUM       17   // Logical Channel  16

#define LAST_PKT     -1

#define IP_PACKET_HEADER_LEN 20
#define IP_ADDRESS_OFFSET    12
#define ARP_IP_ADDRESS_OFFSET 28   


/***** define logical channel states:
	P1     : Ready  ------------------>READY
	P2     : DTE waiting ------------->DTE_WAIT
	P3     : DCE waiting ------------->DCE_WAIT
	P4(=D1): Data transfer ----------->DATA_TRANS
	P5     : Call collision ---------->CALL_COLI
	P6     : DTE clear request ------->CLR_REQ
	P7     : DCE clear indication ---->CLR_IND

	D1(=P4): Flow conttrol ready ----->FLOW_CTL
	D2     : DTE reset request ------->RES_REQ
	D3     : DCE reset indication ---->RES_IND

	R1     : Packet level ready ------>PKT_READY
	R2     : DTE restart request ----->RST_REQ
	R3     : DCE restart indication -->RST_IND
	       : Logical channel error --->LC_ERROR
*******/
#define READY       11
#define DTE_WAIT    12
#define DCE_WAIT    13
#define DATA_TRANS  14
#define CALL_COLI   15
#define CLR_REQ     16
#define CLR_IND     17

#define FLOW_CTL    14
#define RES_REQ     22
#define RES_IND     23

#define PKT_READY   11
#define RST_REQ     32
#define RST_IND     33

#define LC_ERROR    -1

// Define the various timer value
#define CALL_TIMEOUT  200000   // 200 seconds T21
#define CLEAR_TIMEOUT 180000   // 180 seconds T?
#define DATA_PACKET_TIMEOUT  120000  //120 seconds temperily

/***** Define data specifications:
******/
/*** normal user data ***/
#define T_NORMAL     0

/*** expedited data ***/
#define T_EXPEDITED  1

/*** ??? ***/
#define T_MORE       2



/***** Define packet types:
******/
	      /*** call set_up packet ***/
/*  call_request_packet,  incoming_call_packet
    == 0000 1011 */
#define CALL_REQ         0x0b
#define INCOM_CALL       CALL_REQ
/*  call_connected_packet,  call_accept_packet
    == 0000 1111 */
#define CALL_CONN        0x0f
#define CALL_ACPT        CALL_CONN

	      /*** data packet ***/
/*  DTE_data_packet,  DCE_data_packet
    == xxxx xxx0 */
#define DTE_DATA         0x0
#define DCE_DATA         DTE_DATA

		/*** flow control packet ***/
/*  DTE_RR_packet,  DCE_RR_packet
    == xxx0 0001(module 8) or 0000 0001(module 128) */
#define DTE_RR           0x01
#define DCE_RR           DTE_RR
/*  DTE_RNR_packet,  DCE_RNR_packet
    == xxx0 0101(module 8) or 0000 0101(module 128) */
#define DTE_RNR          0x05
#define DCE_RNR          DTE_RNR
/*  DTE_REJ_packet,
    == xxx0 1001(module 8) or 0000 1001(module 128) */
#define DTE_REJ          0x09

	      /*** interrupt packet ***/
/*  DTE_interrupt_packet, DCE_interrupt
    == 0010 0011 */
#define DTE_INT          0x23
#define DCE_INT          DTE_INT
/*  DTE_interrupt_confirmation_packet,
    DCE_interrupr_confirmation_packet
    == 0010 0111 */
#define DTE_INT_CONF     0x27
#define DCE_INT_CONF     DTE_INT_CONF

	      /*** reset & restart packet ***/
/*  reset_request_packet, reset_indicatiopn_packet
    == 0001 1011 */
#define RESET_REQ        0x1b
#define RESET_IND        RESET_REQ
/*  DTE_reset_confirmation_packet,
    DCE_reset_confirmation_packet
    == 0001 1111 */
#define DTE_RES_CONF     0x1f
#define DCE_RES_CONF     DTE_RES_CONF
/*  restart_requset_packet,  restart_indication_packet
    == 1111 1011 */
#define RESTART_REQ      0xfb
#define RESTART_IND      RESTART_REQ
/*  DTE_restart_confirmation_packet,
    DCE_restart_confirmation_packet
    == 1111 1111 */
#define DTE_RST_CONF     0xff
#define DCE_RST_CONF     DTE_RST_CONF

	       /*** call clearing packet ***/
/*  clear_request_packet, clear_indication_packet
    == 0001 0011 */
#define CLEAR_REQ        0x13
#define CLEAR_IND        CLEAR_REQ
/*  DTE_clear_confirmation_packet,
    DCE_clear_confirmation_packet
    == 0001 0111 */
#define DTE_CLR_CONF     0x17
#define DCE_CLR_CONF     DTE_CLR_CONF

	       /*** registration & diagnostic packet ***/
/*  registration_request_packet
    == 1111 0011 */
#define REG_REQ          0xf3
/*  registration_confirmation_packet
    == 1111 0111 */
#define REG_CONF         0xf7
/*  diagnostic_packet
    == 1111 0001 */
#define DIAG             0xf1




// Const defination of the X25 automation 
// These consts are defined in Annex E to 
// Recommendation X.25
// Const defined by Jimmy Guo 
#define InValidPs          1    // Diag code for invalid ps
#define InValidPr          2	// Diag code for invalid pr
#define PacketTooShort     38   // Diag code for packet too short
#define PacketTooLong      39     // Diag code for packet too long 



#define FDUNUSED -1
#define FDUNOPENED FDUNUSED
#define FDCLOSED   FDUNUSED
#define FDUNBIND -2
#define FDUNCONNECTED -3

#define InitSucc    0
#define BindSucc    0
#define ListenSucc  0
#define ConnectSucc 0
#define AcceptSucc  0
#define ClearSucc   0
#define CloseSucc   0
#define RestartSucc 0
#define GetPktSucc  0

#define CALLPOINT   0
#define CALLEDPOINT 1

/***********************************************
	  tli_return_value:: error_code
************************************************/
		      /*** defined by Ling G. Sun ***/
#define  NORMAL 0

#define  LinkInitErr       -1
#define  InitFail          -2
#define  InitFileErr       -3

#define  OpenFail          -20
#define  PktLelBusy        -21
#define  PktLelNotReady   -22


#define  BindFail          -30
#define  NoPVC             -31
#define  ParamErr          -32
#define  FdStateErr        -33

#define  ConnectFail       -41
#define  LCNCleared        -42
#define  LCNErrOccur       -43
#define  T21TimeOut        -44

#define  ListenFail        -51

#define  AcceptFail        -61

#define  T23TimeOut        -71
#define  SnddisFail        -72
#define  ClearFail         -73

#define  RcvdisFail        -81

#define CloseFail          -91

#define SndRstFail         -95
#define T20TimeOut         -96


		      /*** defined by Xin B. Song ***/
/*** definitions used for init.PktLevState,
     i.e. packet_level error state definitions:  ***/
#define PKT_LEV_ERR    -100
       /* buffer pool overflow */
#define POOL_OVER      -101
       /* link_level fail */
#define LLI_FAIL       -102
       /* procedure error */
#define PROC_ERR        -103
      /* received data error(including packet header error) */
#define DATA_ERR        -104

#define RESTARTING      -105

/*** definitions used for function return value ***/
#define ERR_HEADER      -110
#define ERR_QBIT        -111
#define ERR_DBIT        -112
#define ERR_MODUL       -113
#define ERR_LCGN        -114
#define ERR_LCN         -115

#define ERR_PARAM        -121
#define ERR_PKT_TYP      -122

#define QUEUE_ERR        -130
#define QUE_EMPTY        -131
#define NONE_PKT         -132

#define DATA_OVER        -141
#define EXDATA_OVER      -142

#define NOT_P4_STATE      -151

#define CONF_FAIL         -161

/*fd state for pctcp*/
#define TCP_CONNECT      2
#define TCP_WAIT         1

#define FLAG_OFF   8
#endif
