#ifndef  _MPCCCARD_
#define  _MPCCCARD_

#ifndef	u_char
#define	u_char	UCHAR
#endif

#ifndef	u_long
#define	u_long	ULONG
#endif

#ifndef	u_short
#define	u_short USHORT
#endif

/*
i2scard.h
for communication with card
*/
#define	CARD_WIN	1
#define	CARD_FLAT	2

/* MEMORY accessed by local CPU(80186)	*/
#define	LMSEG		0x0000
#define	MM0SEG		0x4000
#define	MM1SEG		0x5000
#define	MM2SEG		0x6000
#define	UMSEG		0xF000

#define	MEM_OK		0x100
/*
#define	LD_WAIT		0x200
#define	LOAD_OK		0x400
*/
#define	LMOK		0x0001
#define	MM0OK		0x0002
#define	MM1OK		0x0004
#define	MM2OK		0x0008
#define	UMOK		0x0010
#define	ERR		0x800
#define	LMERR		(ERR | LMOK)
#define	MM0ERR		(ERR | MM0OK)
#define	MM1ERR		(ERR | MM1OK)
#define	MM2ERR		(ERR | MM2OK)
#define	UMERR		(ERR | UMOK)

#define RUNNING		0x100
#define	I2SWIN_SIZE	(0x4000)	/* 16KB.	*/

/*
host.h
for download software
*/
/* I2S DPRAM download main program head struct.	*/
#include <pshpack1.h>
struct dhead{
	unsigned short	d_cmd;		/* download command */
	unsigned short	d_start;	/* main program start address */
	unsigned short	d_txtlen;	/* text segment size */
	unsigned short	d_datoff;	/* offset of data segment in DPRAM */
	unsigned short	d_datlen;	/* data segment length */
	unsigned short	d_bsslen;	/* bss segment length */
	unsigned short	d_stklen;	/* stack segment length */
	char		d_cpuclk;	/* cpu clock.		*/
	char		d_sccclk;	/* scc clock.		*/
};
#include <poppack.h>
/* I2S CLOCK */
#define	C4MHZ	4
#define	C6MHZ	6
#define	C8MHZ	8
#define	C10MHZ	10
#define	C12MHZ	12

#define MAXCARD		4			/* Max number of cards.	*/
#define NUMPORT		2			/* Number of ports per card.*/
#define MAXPORT		(MAXCARD * NUMPORT)	/* Max number of ports.	*/
#define BUFSZ		8192			/* Output buffer size.	*/
#define BUFMASK		0x1FFF			/* Output buffer mask.	*/
#define NUM_FRM		BUFSZ/(sizeof struct sdlc_buf)


#define	SDLC_FLAG	0x7E
#define FRMLTH		2 + 256

#define	I2SLDOFF	0xFC00	/* Offset of minit in DPRAM.	*/
#define	I2SFLATOFF	0xC000	/* Offset of minit in DPRAM.	*/
#define I2SLDSIZ	0x0400	/* Size   of minit in DPRAM.	*/

#define RUNNING		0x100

#define LD_CMD		0x1000

/* Addition command defination.	*/
#define	T_MINIT		0x80	/* I2S card reset.	*/
#define	T_RESET		0x81	/* I2S card reset only.	*/

/* For bisdpram download software.	*/
#define DPRAMDEV	0x00FC
#define DPRAMCDN	0x0003

/* For i2sminit() flag.	*/
#define	I2SMINIT	0x0001	/* MAPPING	*/
#define	I2STEST		0x0002	/* I2S card test	*/

/* For i2s card intrrupt	*/
#define	NUM_VEC		8	/* Number of selectable intrrupt vector	*/
#ifdef	SCOUNIX
#define	INT3		3
#define	INT4		4
#define	INT5		5
#define	INT9		9
#define	INT10		10
#define	INT11		11
#define	INT12		12
#define	INT15		15
#endif
#ifdef	SCOXENIX
#define	INT3		3
#define	INT4		4
#define	INT5		5
#define	INT9		25
#define	INT10		26
#define	INT11		27
#define	INT12		28
#define	INT15		31
#endif

/*
mytype.h
for test ioctl
*/

#define T_DLOAD1    0x800
#define T_DLOAD2    0x801
#define T_SDLOAD    0x802
#define T_EDLOAD    0x803
#define T_CDLOAD    0x804
#define T_TRACE     0x805
#define L_TEST      0x806
#define ST_TEST     0x807
#define MEM_TEST    0x808
#define L_REBOOT    0x809
#define CMD_TEST    0x80a
#define T_SETACTIVE 0x80b

#define BUFSIZE 1024
struct  I2Sbuf{
	int length;
	unsigned char   cbuf[BUFSIZE];
};
struct  test{
	unsigned char   a;
	unsigned char   b;
};
struct  mtestbuf{
	int addr;
	unsigned char   buf[BUFSIZE];
	int offset;
};

/*
from pc2card.h
*/

#define	BUFLEN	1024
#define	RXBUF_LEN	BUFLEN
#define	TXBUF_LEN	BUFLEN
#define	INFOBUF_LEN	BUFLEN

/*host cmd*/

#include <pshpack1.h>

#define NMBLOCKS	100		
#define FRMSIZ		((16+2)*8)
#define BUFNUM		128	
#define TESTBUFSIZE	4000

typedef struct mblock{
/*
	struct mblock	*mb_next;
*/
	USHORT	mb_next;
	USHORT	mb_offset;
	USHORT	mb_bufsize;
	UCHAR	mb_flag;
	UCHAR	PADDING;
	USHORT	mb_mseg;
	USHORT	mb_moff;
} mblock_t;

/*
typedef mblock_t	*mblockp_t;
*/
typedef USHORT	mblockp_t;

typedef struct {
        USHORT         mb_list_count;  /* mb list count */
        USHORT         mb_list_first;  /* pointer to first */
        USHORT         mb_list_last;   /* pointer to last */
} mb_list_t;
/*
typedef mb_list_t  *mb_listp_t;
*/
typedef USHORT mb_listp_t;

/* mblock flag */
#define MB_FREE_FRM	0x0
#define MB_MIDDLE_FRM	0x0
#define MB_FIRST_FRM	0x1
#define MB_FINAL_FRM	0x2

/* SND,RCV FLAG */
#define SEND_XQ_FIRST	1
#define SEND_XQ_FINAL	2
#define RECV_RQ_FIRST	1
#define RECV_RQ_FINAL	2
#define KEEP		4

//#define get_mbp(Adapter,x)      ((x==0)?0:((mblock_t *)((PUCHAR)(Adapter->VirtualAddress)+READ_REGISTER_USHORT(&(x)))))
//#define get_mlistp(Adapter,x)   ((x==0)?0:((mb_list_t *)((PUCHAR)(Adapter->VirtualAddress)+READ_REGISTER_USHORT(&(x)))))
//REPLACED BY FANG AS:

/*mblock_t *get_mbp(IN PNE2000_ADAPTER       Adapter,
                  IN USHORT x
                  );
 
mb_list_t *  get_mlistp(
   IN  PNE2000_ADAPTER Adapter,
   IN  USHORT  x
   ); 
 */

typedef	struct	{
	UCHAR	link_type[NUMPORT];
#define	LINK_NOTUSED	0x0
#define	LAPB		0x1
#define	SDLC_PRI	0x2
#define	SDLC_SEC	0x3
#define	SDLC_COM	0x4
#define	SDLC_3270	0x5
#define	LINK_RAW	0x6

	UCHAR	portmod[NUMPORT];
#define	CLK_OFF		0x1
#define	CLK_INT		0x0
#define	CLK_EXT		0x1
#define	DEV_OFF		0x2
#define	DEV_DCE		0x0
#define	DEV_DTE		0x2
#define	MODUL_OFF	0x4
#define	MODUL_8		0x0
#define	MODUL_128	0x4
#define	CODE_OFF	0x18
#define	CODE_NRZ	0x0
#define	CODE_NRZI	0x8
#define	CODE_FM		0x10
#define	DPX_OFF		0x20
#define	HALF_DPX	0x0
#define	FULL_DPX	0x20
	UCHAR	K[NUMPORT];
	UCHAR	N2[NUMPORT];
	UCHAR	T1[NUMPORT];
/*SDLC ONLY*/
	UCHAR	link_addr[NUMPORT];
	UCHAR	rej_used[NUMPORT];

	USHORT	max_frame_len[NUMPORT];
	UCHAR	port_state[NUMPORT];
#define	CH_OFF		0x3
#define	CH_DOWN	0x0
#define	CH_OK	0x1
#define	CH_ERR	0x2

#define	RX_OFF	0xc
#define	RX_DOWN	0x0
#define	RX_INIT		0x4
#define	RX_OK		0x8
#define	RX_ERR		0xc

#define	TX_OFF	0x30
#define	TX_DOWN	0x0
#define	TX_INIT		0x10
#define	TX_OK		0x20
#define	TX_ERR		0x30

#define	TXFSM_OFF	0xc0
#define	TXFSM_NORMAL	0x00
#define	TXFSM_INFOEND	0x40
#define	TXFSM_TXBRK	0x80
#define	TXFSM_STOP	0xc0

#define	CH_START	0x0
#define	RX_START	0x1
#define	TX_START	0x2
#define	STATE_INTR	0x3

	UCHAR	link_state[NUMPORT];
#define	LINK_NOSTATE	0x0
#define	LINK_DISCONN	0x1
#define	LINK_NORMAL	0x2
#define	LINK_TRANSIENT	0x3
#define	LINK_ACTIVE	0x4
	UCHAR	ctll_state[NUMPORT];
	UCHAR	rctl_ok[NUMPORT];
	UCHAR	wctl_ok[NUMPORT];
	UCHAR	baudrate[NUMPORT];

/*getting from termio.h*/
#define	B0	0
#define	B50	001
#define	B75	002
#define	B110	003
#define	B134	004
#define	B150	005
#define	B200	006
#define	B300	007
#define	B600	010
#define	B1200	011
#define	B1800	012
#define	B2400	013
#define	B4800	014
#define	B9600	015
#define	B19200	016
/* Addition baud rate defination for SYNC.(Addition control mode)	*/
#define	B38400		017		/* Equal CS6	*/
#define	B64000		020		/* Equal CS6	*/
#define B128000		021		/* Equal CS6 | B50	*/
#define B256000		022		/* Equal CS6 | B75	*/
#define B512000		023		/* Equal CS6 | B110	*/
#define B1000000	024		/* Equal CS6 | B134	*/

	UCHAR	timehigh[NUMPORT];
	UCHAR	timelow[NUMPORT];
/*STRUCT FOR EXCHANGE*/
	UCHAR	intrflag;
#define	RXDATA0_RDY	1
#define	RXDATA1_RDY	2
#define	TX0_FINISH	3
#define	TX1_FINISH	4
#define	LOCAL0_RDY	5
#define	LOCAL1_RDY	6
#define	LEXP0_RDY	7
#define	LEXP1_RDY	8
#define	LPRINT		9
#define	LINK_ST0CHANGE	10
#define	LINK_ST1CHANGE	11
	UCHAR	intrack;

	UCHAR	local_rdy[NUMPORT];	/*1 local ready,host can send cmd*/
	UCHAR	host_cmd[NUMPORT];
#define	TXDATA_RDY	0
#define	LINKCMD_CONN	1
#define	LINKCMD_DISCONN	2
#define	LINKCMD_GETST	3
#define	LINKCMD_SETOK	4
	UCHAR	lexp_rdy[NUMPORT];	/*1 local ready,host can send exceptional cmd*/
	UCHAR	hexp_cmd[NUMPORT];
#define	RX_FINISH	0


	UCHAR	Rxdata_rdy[NUMPORT];
	mb_listp_t	Rxdata_ptr[NUMPORT];

	UCHAR	Txdata_rdy[NUMPORT];
	mb_listp_t	Txdata_ptr[NUMPORT];

#define	MAXXID	80
	UCHAR	xid[NUMPORT][MAXXID];
	UCHAR	xidlen[NUMPORT];
	UCHAR	pstr[2];
	UCHAR	intoverflow;
	UCHAR	intr_na;
	UCHAR	parmrcnt;
	UCHAR	parmwcnt;
	UCHAR	hostint;
	UCHAR	sccintr;
	USHORT	und_eom[NUMPORT];
	USHORT	brk_abrt[NUMPORT];
	USHORT	crc_err[NUMPORT];
	USHORT	over_err[NUMPORT];
	UCHAR	rxintr[NUMPORT];
	UCHAR	txintr[NUMPORT];
	UCHAR	esintr[NUMPORT];
	UCHAR	cmdgetok[NUMPORT];
	UCHAR	putover[NUMPORT];
}pc2card_scb_t;
#include <poppack.h>


#define	sdlc_s2b(x,y)	((y)[0]=(UCHAR)((x)/256),(y)[1]=(UCHAR)((x)%256))
#define	sdlc_b2s(x,y)	(y=(UCHAR)x[0]*256+(UCHAR)x[1])

//#define USE_LEASED_LINE 1
#define	HTDSU_MEMORY_SIZE	0x4000

#endif
