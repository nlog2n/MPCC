
/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    card.c

Abstract:

    Card-specific functions for the NDIS 3.0 Novell 2000 driver.

Author:

    

Environment:

    Kernel mode, FSD

Revision History:

--*/

#include <ndis.h>
#include "mpcccard.h"
#include "ne2000hw.h"
#include "ne2000sw.h"
#include "mpccfunc.h"
#include "Debug.h"
#include "typdef.h"


VOID HtDsuHandlePacket(
	IN PNE2000_ADAPTER   Adapter,
	IN PUCHAR           Data_buf,
	IN UINT             Data_len
    );


//by fang

//by fang is end

#define  __FILEID__     2       // Unique file ID for error logging

/*THIS DEFINES ARE FOR WIN CARD*/
#define I2SADL_R        0
#define I2SADH_R        2
#define WININT_R        4

#define I2SADL_BITS     0xf
#define AWIN_SHIFT      0x4
#define AWIN_BITS       0xf0

#define I2SADH_BITS     0x3f
#define AH_SHIFT        0x4
#define I2SRES_BIT      0x80
#define I2S_RESET       0x00
#define I2S_WORK        0x80
#define I2SMEM_BIT      0x40
#define I2SMEM_CLOSE    0x00
#define I2SMEM_OPEN     0x40

#define WININT_RST      0x00
#define WININT_WORK     0x80
#define WININT_RSTBIT   0x80
#define WININT_INT      0x40
#define WININT_INTBIT   0x40

/*THIS DEFINES ARE FOR FLAT CARD*/
#define I2SADDR_R       0
#define FLAT_SHIFT      3
#define I2SRES_R        6
#define FLAT_EN 0x80
#define INTSEL_R        2
#define FLATINT_R       4

#define DPSHIFT         0xe             /* x 16KB.              */
#define FLAT_MEM        (0x20000)          /* 128KB.       */
#define MAXCARD                 4
#include "loadwin.h"
#include "software.h"

UCHAR	confbuf[sizeof(pc2card_scb_t)+sizeof(ULONG)];


void i2swininit(IN PNE2000_ADAPTER       Adapter)
{
	Adapter->savewin=0;
}

void i2sreset(IN PNE2000_ADAPTER       Adapter)
{
	NdisRawWritePortUchar(Adapter->IoPAddr+WININT_R,(unsigned char)(WININT_RSTBIT&WININT_RST));
}

void i2sintr(IN PNE2000_ADAPTER       Adapter)
{
	NdisRawWritePortUchar(Adapter->IoPAddr+WININT_R,
			   (unsigned char) (WININT_RSTBIT&WININT_WORK)|(WININT_INTBIT&WININT_INT));
}

int	licopyin(PUCHAR src,PUCHAR	dec,ULONG	len)	
{
	UINT	i;
		for(i=0;i<len;i++){
		dec[i]=src[i];
	}
	return(0);
}	

#define	NE2000_DPRAM(Adapter)	((NdisGetPhysicalAddressLow(Adapter->PhysicalAddress))>>14)

int	i2ssetwinsave(IN PNE2000_ADAPTER       Adapter)
{
    if(Adapter->savewin!=0){
		return(1);
	}
    
    Adapter->savewin=Adapter->currentwin;
    Adapter->currentwin=0xc;
    NdisRawWritePortUchar(Adapter->IoPAddr+I2SADL_R,
		(unsigned char)(NE2000_DPRAM(Adapter)&I2SADL_BITS)|((Adapter->currentwin<<AWIN_SHIFT)&AWIN_BITS));
    return(0);
}

void i2sresetwin(IN PNE2000_ADAPTER       Adapter)
{
	Adapter->currentwin=Adapter->savewin;
	NdisRawWritePortUchar(Adapter->IoPAddr+I2SADL_R,
		(unsigned char)(NE2000_DPRAM(Adapter)&I2SADL_BITS)|((Adapter->currentwin<<AWIN_SHIFT)&AWIN_BITS));
	Adapter->savewin=0;
}

PVOID i2sf2w(IN PNE2000_ADAPTER       Adapter,PVOID offset)
{
	unsigned long real_off;

	real_off=(PUCHAR)(offset) - (PUCHAR)(Adapter->VirtualAddress);
	if(Adapter->currentwin != (0xc+(UCHAR)(real_off/I2SWIN_SIZE))){
	    Adapter->currentwin = (0xc+(UCHAR)(real_off/I2SWIN_SIZE));
	    NdisRawWritePortUchar(Adapter->IoPAddr+I2SADL_R,
			(unsigned char)(NE2000_DPRAM(Adapter)&I2SADL_BITS)|((Adapter->currentwin<<AWIN_SHIFT)&AWIN_BITS));
		}
	return((PVOID)((PUCHAR)(Adapter->VirtualAddress) + real_off % I2SWIN_SIZE) );
}

int i2ssetwin(IN PNE2000_ADAPTER       Adapter, unsigned short segment, 
unsigned short offset)
{

	DBG_FUNC("i2ssetwin")

	register        unsigned char  start;

	switch(segment){
	case MM0SEG:
		start=0;
		break;
	case MM1SEG:
		start=4;
		break;
	case MM2SEG:
		start=8;
		break;
	case UMSEG:
		start=0xc;
		break;
	default:
		DBG_ERROR(Adapter,("i2ssetwin segerr:%x\n",segment));
		return(-1);
	}
	Adapter->currentwin=start+(unsigned char)(offset/I2SWIN_SIZE);
	NdisRawWritePortUchar(Adapter->IoPAddr+I2SADL_R,
		(unsigned char)(NE2000_DPRAM(Adapter)&I2SADL_BITS)|((Adapter->currentwin<<AWIN_SHIFT)&AWIN_BITS));
	return(0);
}


VOID MyUnicodeToAnsi(MY_ANSISTRING *mystring,
                     NDIS_STRING *ustring);
VOID GetAddrPair(ADDR_PAIR_STRING *mystring,
                 NDIS_STRING *ustring
                     );
                     
//added by fang hui on 11 13
mblock_t *get_mbp(IN PNE2000_ADAPTER       Adapter,
                  IN USHORT x
         )
{
  USHORT  ush;
  if(x==0) return 0;
  
  NdisReadRegisterUshort(&x,&ush);
  return ( (mblock_t *)((PUCHAR)(Adapter->VirtualAddress)+ush )  );
}  

mb_list_t *get_mlistp(
   IN  PNE2000_ADAPTER Adapter,
   IN  USHORT  x
   ) 
{ 
 USHORT uch;
 if(x==0) return 0;
 NdisReadRegisterUshort(&x,&uch);
 return ( (mb_list_t *)((PUCHAR)(Adapter->VirtualAddress)+uch) );
}
/////////////////////

void Cardconf(NDIS_HANDLE ConfigHandle, pc2card_scb_t *pc2card_scbp, ULONG *len)
{

	DBG_FUNC("Cardconf")
	
    PNDIS_CONFIGURATION_PARAMETER ReturnedValue;
    NDIS_STATUS Status;

    NDIS_STRING LINKADDR0String = NDIS_STRING_CONST("LINKADDR0");
    NDIS_STRING LINKADDR1String = NDIS_STRING_CONST("LINKADDR1");

    NDIS_STRING LINKN20String = NDIS_STRING_CONST("LINKN20");
    NDIS_STRING LINKN21String = NDIS_STRING_CONST("LINKN21");

    NDIS_STRING LINKT10String = NDIS_STRING_CONST("LINKT10");
    NDIS_STRING LINKT11String = NDIS_STRING_CONST("LINKT11");

    NDIS_STRING FRAMELEN0String = NDIS_STRING_CONST("FRAMELEN0");
    NDIS_STRING FRAMELEN1String = NDIS_STRING_CONST("FRAMELEN1");

    NDIS_STRING BAUDRATE0String = NDIS_STRING_CONST("BAUDRATE0");
    NDIS_STRING BAUDRATE1String = NDIS_STRING_CONST("BAUDRATE1");

    NDIS_STRING LINKTYPE0String = NDIS_STRING_CONST("LINKTYPE0");
    NDIS_STRING LINKTYPE1String = NDIS_STRING_CONST("LINKTYPE1");

    NDIS_STRING LINKCLK0String = NDIS_STRING_CONST("LINKCLK0");
    NDIS_STRING LINKCLK1String = NDIS_STRING_CONST("LINKCLK1");

    NDIS_STRING LINKDEV0String = NDIS_STRING_CONST("LINKDEV0");
    NDIS_STRING LINKDEV1String = NDIS_STRING_CONST("LINKDEV1");

    NDIS_STRING LINKMODUL0String = NDIS_STRING_CONST("LINKMODUL0");
    NDIS_STRING LINKMODUL1String = NDIS_STRING_CONST("LINKMODUL1");

    NDIS_STRING COMMCODE0String = NDIS_STRING_CONST("COMMCODE0");
    NDIS_STRING COMMCODE1String = NDIS_STRING_CONST("COMMCODE1");

    NDIS_STRING LINKWIN0String = NDIS_STRING_CONST("LINKWIN0");
    NDIS_STRING LINKWIN1String = NDIS_STRING_CONST("LINKWIN1");

	NdisZeroMemory(pc2card_scbp,sizeof(pc2card_scb_t));
	pc2card_scbp->baudrate[0]=B9600;
	pc2card_scbp->baudrate[1]=B9600;
	
	pc2card_scbp->link_addr[0]=0x1;
	pc2card_scbp->link_addr[1]=0x1;
	pc2card_scbp->link_type[0]=LAPB;
	pc2card_scbp->link_type[1]=LAPB;
	pc2card_scbp->portmod[0]=CLK_EXT|DEV_DTE|MODUL_8|CODE_NRZ|FULL_DPX;
	pc2card_scbp->portmod[1]=CLK_INT|DEV_DCE|MODUL_8|CODE_NRZ|FULL_DPX;
	pc2card_scbp->K[0]=30;
	pc2card_scbp->K[1]=30;
	pc2card_scbp->N2[0]=5;
	pc2card_scbp->N2[1]=5;
	pc2card_scbp->T1[0]=60;
	pc2card_scbp->T1[1]=60;
	pc2card_scbp->rctl_ok[0]=0;
	pc2card_scbp->rctl_ok[1]=0;
	pc2card_scbp->wctl_ok[0]=0;
	pc2card_scbp->wctl_ok[1]=0;
	pc2card_scbp->max_frame_len[0]=282;
	pc2card_scbp->max_frame_len[1]=282;
	pc2card_scbp->rej_used[0]=0;
	pc2card_scbp->rej_used[0]=0;

    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &BAUDRATE0String,
	    NdisParameterHexInteger
	    );

    if (Status == NDIS_STATUS_SUCCESS) {
		ULONG	baudvalue;
		baudvalue=(ReturnedValue->ParameterData.IntegerData);
		//modified by fang
		if(baudvalue==0x9600){
			pc2card_scbp->baudrate[0]=B9600;
		}
		else if(baudvalue==0x19200){
			pc2card_scbp->baudrate[0]=B19200;
		}
		else if(baudvalue==0x38400){
			pc2card_scbp->baudrate[0]=B38400;
		}
		else if(baudvalue==0x64000){
			pc2card_scbp->baudrate[0]=B64000;
		}
		else{
			DBG_DISPLAY(("unknow baudrate0==%d\n",baudvalue));
		}
    }
    else{
    	DBG_DISPLAY(("read BAUDRATE0 error :%d\n",Status));
    }

    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &BAUDRATE1String,
	    NdisParameterHexInteger
	    );

    if (Status == NDIS_STATUS_SUCCESS) {
		ULONG	baudvalue;
		baudvalue=(ReturnedValue->ParameterData.IntegerData);
		if(baudvalue==0x9600){
			pc2card_scbp->baudrate[1]=B9600;
		}
		else if(baudvalue==0x19200){
			pc2card_scbp->baudrate[1]=B19200;
		}
		else if(baudvalue==0x38400){
			pc2card_scbp->baudrate[1]=B38400;
		}
		else if(baudvalue==0x64000){
			pc2card_scbp->baudrate[1]=B64000;
		}
		else{
			DBG_DISPLAY(("unknow baudrate1==%d\n",baudvalue));
		}
    }
    else{
    	DBG_DISPLAY(("read BAUDRATE0 error :%d\n",Status));
    }

    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &LINKADDR0String,
	    NdisParameterHexInteger
	    );

    if (Status == NDIS_STATUS_SUCCESS) {
    	pc2card_scbp->link_addr[0]=(UCHAR)(ReturnedValue->ParameterData.IntegerData);
	}
	else{
		DBG_DISPLAY(("read LINKADDR0 error:%d\n",Status));
	}
	
    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &LINKADDR1String,
	    NdisParameterHexInteger
	    );

    if (Status == NDIS_STATUS_SUCCESS) {
    	pc2card_scbp->link_addr[1]=(UCHAR)(ReturnedValue->ParameterData.IntegerData);
	}
	else{
		DBG_DISPLAY(("read LINKADDR0 error:%d\n",Status));
	}

    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &LINKN20String,
	    NdisParameterHexInteger
	    );

    if (Status == NDIS_STATUS_SUCCESS) {
    	pc2card_scbp->N2[0]=(UCHAR)(ReturnedValue->ParameterData.IntegerData);
	}
	else{
		DBG_DISPLAY(("read LINKN20 error:%d\n",Status));
	}
	
	
    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &LINKN21String,
	    NdisParameterHexInteger
	    );

    if (Status == NDIS_STATUS_SUCCESS) {
    	pc2card_scbp->N2[1]=(UCHAR)(ReturnedValue->ParameterData.IntegerData);
	}
	else{
		DBG_DISPLAY(("read LINKN21 error:%d\n",Status));
	}
	
    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &LINKWIN0String,
	    NdisParameterHexInteger
	    );

    if (Status == NDIS_STATUS_SUCCESS) {
        UCHAR   uch;
        uch=(UCHAR)(ReturnedValue->ParameterData.IntegerData);
        if(uch>0)
    	pc2card_scbp->K[0]=uch;
	}
	else{
		DBG_DISPLAY(("read LINKWIN0 error:%d\n",Status));
	}
	
    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &LINKWIN1String,
	    NdisParameterHexInteger
	    );

    if (Status == NDIS_STATUS_SUCCESS) {
    	pc2card_scbp->K[1]=(UCHAR)(ReturnedValue->ParameterData.IntegerData);
	}
	else{
		DBG_DISPLAY(("read LINKWIN1 error:%d\n",Status));
	}
	
    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &LINKT10String,
	    NdisParameterHexInteger
	    );

    if (Status == NDIS_STATUS_SUCCESS) {
    	pc2card_scbp->T1[0]=(UCHAR)(ReturnedValue->ParameterData.IntegerData);
	}
	else{
		DBG_DISPLAY(("read LINKT10 error:%d\n",Status));
	}
	
    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &FRAMELEN0String,
	    NdisParameterHexInteger
	    );

    if (Status == NDIS_STATUS_SUCCESS) {
        UCHAR  uch;
        uch=(UCHAR)(ReturnedValue->ParameterData.IntegerData);
        if(uch>0)	
    	pc2card_scbp->max_frame_len[0]=uch;
	}
	else{
		DBG_DISPLAY(("read FRAMELEN0 error:%d\n",Status));
	}
	
    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &FRAMELEN1String,
	    NdisParameterHexInteger
	    );

    if (Status == NDIS_STATUS_SUCCESS) {
    	pc2card_scbp->max_frame_len[1]=(UCHAR)(ReturnedValue->ParameterData.IntegerData);
	}
	else{
		DBG_DISPLAY(("read FRAMELEN1 error:%d\n",Status));
	}
	
    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &LINKT11String,
	    NdisParameterHexInteger
	    );

    if (Status == NDIS_STATUS_SUCCESS) {
    	pc2card_scbp->T1[1]=(UCHAR)(ReturnedValue->ParameterData.IntegerData);
	}
	else{
		DBG_DISPLAY(("read LINKT11 error:%d\n",Status));
	}
	
	
    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &LINKTYPE0String,
	    NdisParameterString
	    );

    if (Status == NDIS_STATUS_SUCCESS)
    {
          MY_ANSISTRING  LINKTYPE;
       ////////////
        MyUnicodeToAnsi(&LINKTYPE,
	     &(ReturnedValue->ParameterData.StringData) );
		
		if(strcmp(LINKTYPE.Buffer,"LAPB")==0){
			pc2card_scbp->link_type[0]=LAPB;
		}
		else if(strcmp(LINKTYPE.Buffer,"LINK_NOTUSED")==0){
			pc2card_scbp->link_type[0]=LINK_NOTUSED;
		}
		else if(strcmp(LINKTYPE.Buffer,"SDLC_PRI")==0){
			pc2card_scbp->link_type[0]=SDLC_PRI;
		}
		else if(strcmp(LINKTYPE.Buffer,"SDLC_SEC")==0){
			pc2card_scbp->link_type[0]=SDLC_SEC;
		}
		else{
			DBG_DISPLAY(("unknow link_type0==%s\n",
				LINKTYPE.Buffer));
		}
    }
	else{
		DBG_DISPLAY(("read LINKTYPE0 error:%d\n",Status));
	}


    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &LINKTYPE1String,
	    NdisParameterString
	    );

    if (Status == NDIS_STATUS_SUCCESS)
    {
        MY_ANSISTRING LINKTYPE;

	MyUnicodeToAnsi(
		&LINKTYPE,
		&(ReturnedValue->ParameterData.StringData)
		);
		if(strcmp(LINKTYPE.Buffer,"LAPB")==0){
			pc2card_scbp->link_type[1]=LAPB;
		}
		else if(strcmp(LINKTYPE.Buffer,"LINK_NOTUSED")==0){
			pc2card_scbp->link_type[1]=LINK_NOTUSED;
		}
		else if(strcmp(LINKTYPE.Buffer,"SDLC_PRI")==0){
			pc2card_scbp->link_type[1]=SDLC_PRI;
		}
		else if(strcmp(LINKTYPE.Buffer,"SDLC_SEC")==0){
			pc2card_scbp->link_type[1]=SDLC_SEC;
		}
		else{
			DBG_DISPLAY(("unknow link_type1==%s\n",
				LINKTYPE.Buffer));
		}
    }
	else{
		DBG_DISPLAY(("read LINKTYPE1 error:%d\n",Status));
	}


   NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &LINKCLK0String,
	    NdisParameterString
	    );

    if (Status == NDIS_STATUS_SUCCESS)
    {
        MY_ANSISTRING LINKTYPE;

	MyUnicodeToAnsi(
		&LINKTYPE,
		&(ReturnedValue->ParameterData.StringData)
		);
		if(strcmp(LINKTYPE.Buffer,"CLK_INT")==0){
			pc2card_scbp->portmod[0]=
			(pc2card_scbp->portmod[0]&(~CLK_OFF))|CLK_INT;

		}
		else if(strcmp(LINKTYPE.Buffer,"CLK_EXT")==0){
			pc2card_scbp->portmod[0]=
			(pc2card_scbp->portmod[0]&(~CLK_OFF))|CLK_EXT;

		}
		else{
			DBG_DISPLAY(("unknow LINCLK0==%s\n",LINKTYPE.Buffer));
		}
    }
	else{
		DBG_DISPLAY(("read LINKCLK0 error:%d\n",Status));
	}


	
//above no problem


       


	
    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &LINKDEV0String,
	    NdisParameterString
	    );

    if (Status == NDIS_STATUS_SUCCESS)
    {
        MY_ANSISTRING LINKTYPE;

	    MyUnicodeToAnsi(
		&LINKTYPE,
		&(ReturnedValue->ParameterData.StringData)
		);
		if(strcmp(LINKTYPE.Buffer,"DEV_DTE")==0){
			pc2card_scbp->portmod[0]=
			(pc2card_scbp->portmod[0]&(~DEV_OFF))|DEV_DTE;

		}
		else if(strcmp(LINKTYPE.Buffer,"DEV_DCE")==0){
			pc2card_scbp->portmod[0]=
			(pc2card_scbp->portmod[0]&(~DEV_OFF))|DEV_DCE;

		}
		else{
			DBG_DISPLAY(("unknow LINKDEV0==%s\n",LINKTYPE.Buffer));
		}
    }
	else{
		DBG_DISPLAY(("read LINKDEV0 error:%d\n",Status));
	}


    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &LINKMODUL0String,
	    NdisParameterString
	    );

    if (Status == NDIS_STATUS_SUCCESS)
    {
        MY_ANSISTRING LINKTYPE;

	MyUnicodeToAnsi(
		&LINKTYPE,
		&(ReturnedValue->ParameterData.StringData)
		);
		if(strcmp(LINKTYPE.Buffer,"MODUL_8")==0){
			pc2card_scbp->portmod[0]=
			(pc2card_scbp->portmod[0]&(~MODUL_OFF))|MODUL_8;

		}
		else if(strcmp(LINKTYPE.Buffer,"MODUL_128")==0){
			pc2card_scbp->portmod[0]=
			(pc2card_scbp->portmod[0]&(~MODUL_OFF))|MODUL_128;

		}
		else{
			DBG_DISPLAY(("unknow LINKMODUL0==%s\n",
				LINKTYPE.Buffer));
		}
    }
	else{
		DBG_DISPLAY(("read LINKMODUL0 error:%d\n",Status));
	}

    NdisReadConfiguration(
	    &Status,
	    &ReturnedValue,
	    ConfigHandle,
	    &COMMCODE0String,
	    NdisParameterString
	    );

    if (Status == NDIS_STATUS_SUCCESS)
    {
        MY_ANSISTRING LINKTYPE;

	    MyUnicodeToAnsi(
		&LINKTYPE,
		&(ReturnedValue->ParameterData.StringData)
		);
		if(strcmp(LINKTYPE.Buffer,"CODE_NRZ")==0){
			pc2card_scbp->portmod[0]=
			(pc2card_scbp->portmod[0]&(~CODE_OFF))|CODE_NRZ;

		}
		else if(strcmp(LINKTYPE.Buffer,"CODE_NRZI")==0){
			pc2card_scbp->portmod[0]=
			(pc2card_scbp->portmod[0]&(~CODE_OFF))|CODE_NRZI;

		}
		else{
			DBG_DISPLAY(("unknow COMMCODE0==%s\n",
				LINKTYPE.Buffer));
		}
    }
	else{
		DBG_DISPLAY(("read COMMCODE0 error:%d\n",Status));
	}

    pc2card_scbp->intr_na=0;
    *len=sizeof(pc2card_scb_t);

    
}


int	getbaud(UCHAR	baud,UCHAR sccclk,PUCHAR timelow,PUCHAR timehigh)
{
	u_long  f=0;
	u_long  cbaud;
	u_long  time;

	switch(baud){
	case B0:
		cbaud = 0;
		break;
	case B50:
		cbaud = 50;
		break;
	case B75:
		cbaud = 75;
		break;
	case B110:
		cbaud = 110;
		break;
	case B134:
		cbaud = 134;
		break;
	case B150:
		cbaud = 150;
		break;
	case B200:
		cbaud = 200;
		break;
	case B300:
		cbaud = 300;
		break;
	case B600:
		cbaud = 600;
		break;
	case B1200:
		cbaud = 1200;
		break;
	case B1800:
		cbaud = 1800;
		break;
	case B2400:
		cbaud = 2400;
		break;
	case B4800:
		cbaud = 4800;
		break;
	case B9600:
		cbaud = 9600;
		break;
	case B19200:
		cbaud = 19200;
		break;
	case B38400:
		cbaud = 38400L;
		break;
	case B64000:
		cbaud = 64 * 1024L;
		break;
	case B128000:
		cbaud = 128 * 1024L;
		break;
	case B256000:
		cbaud = 256 * 1024L;
		break;
	case B512000:
		cbaud = 512 * 1024L;
		break;
	case B1000000:
		cbaud = 1024 * 1024L;
		break;
	default:
		cbaud = 0;
		break;
	}
	if(cbaud==0){
		return(-1);
	}

	f=1000000L*sccclk;
	time=f/(cbaud*2)-2;
	NdisWriteRegisterUchar((PUCHAR)timelow,(u_char)(time%256));
   	NdisWriteRegisterUchar((PUCHAR)timehigh,(u_char)(time/256));
   	/*
	WRITE_REGISTER_UCHAR(timelow,(u_char)(time%256));
	WRITE_REGISTER_UCHAR(timehigh,(u_char)(time/256));
	*/
	//replaced by fang
	return(0);
}

int pc2card_reset(IN PNE2000_ADAPTER Adapter, PUCHAR v_dpram, unsigned short st)
{
	 int   j;
	 int    k;
	 UCHAR  uCh1,uCh2,uCh3,uCh4;

	Adapter->inread0=0;
	Adapter->inread1=0;
	Adapter->pc2card_scbp=(pc2card_scb_t *)(v_dpram+st);

    NdisWriteRegisterUchar((PUCHAR)&(Adapter->pc2card_scbp->intrack),1);
    NdisWriteRegisterUchar((PUCHAR)&(Adapter->pc2card_scbp->hostint),0);


/*	WRITE_REGISTER_UCHAR(&(Adapter->pc2card_scbp->intrack),1);
	WRITE_REGISTER_UCHAR(&(Adapter->pc2card_scbp->hostint),0);
	*/
	//substituted by fang
	for(k=0;k<NUMPORT;k++){
        NdisReadRegisterUchar(&(Adapter->pc2card_scbp->baudrate[k]),
                            &uCh1);
		getbaud(uCh1,
 			0x8,
    		Adapter->pc2card_scbp->timelow+k,
	    	Adapter->pc2card_scbp->timehigh+k);

        NdisWriteRegisterUchar((PUCHAR)&(Adapter->pc2card_scbp->Txdata_rdy[k]),0);
        NdisWriteRegisterUchar((PUCHAR)&(Adapter->pc2card_scbp->host_cmd[k]),  0);
        NdisWriteRegisterUchar((PUCHAR)&(Adapter->pc2card_scbp->ctll_state[k]),0);
        NdisWriteRegisterUchar((PUCHAR)&(Adapter->pc2card_scbp->und_eom[k]),   0);
        NdisWriteRegisterUchar((PUCHAR)&(Adapter->pc2card_scbp->brk_abrt[k]),  0);
        NdisWriteRegisterUchar((PUCHAR)&(Adapter->pc2card_scbp->crc_err[k]),   0);
        NdisWriteRegisterUchar((PUCHAR)&(Adapter->pc2card_scbp->over_err[k]),  0);
        NdisWriteRegisterUchar((PUCHAR)&(Adapter->pc2card_scbp->link_state[k]),LINK_NOSTATE);
  


/*		
		WRITE_REGISTER_UCHAR(&(Adapter->pc2card_scbp->Txdata_rdy[k]),0);
		WRITE_REGISTER_UCHAR(&(Adapter->pc2card_scbp->host_cmd[k]),0);
		WRITE_REGISTER_UCHAR(&(Adapter->pc2card_scbp->ctll_state[k]),0);
		WRITE_REGISTER_USHORT(&(Adapter->pc2card_scbp->und_eom[k]),0);
		WRITE_REGISTER_USHORT(&(Adapter->pc2card_scbp->brk_abrt[k]),0);
		WRITE_REGISTER_USHORT(&(Adapter->pc2card_scbp->crc_err[k]),0);
		WRITE_REGISTER_USHORT(&(Adapter->pc2card_scbp->over_err[k]),0);
		WRITE_REGISTER_UCHAR(&(Adapter->pc2card_scbp->link_state[k]),LINK_NOSTATE);
*/
//replaced by fang

	}
	i2sintr(Adapter);

   for(j=0;j<2048L*1024L;j++)
   {
     NdisReadRegisterUchar((PUCHAR) &(Adapter->pc2card_scbp->link_type[0]),&uCh1);
  	 NdisReadRegisterUchar((PUCHAR) &(Adapter->pc2card_scbp->link_state[0]),&uCh2);
	 NdisReadRegisterUchar((PUCHAR) &(Adapter->pc2card_scbp->link_type[1]),&uCh3);
  	 NdisReadRegisterUchar((PUCHAR) &(Adapter->pc2card_scbp->link_state[1]),&uCh4);

  	 if( !(
  	         (uCh1!=LINK_NOTUSED && uCh2==LINK_NOSTATE)
  	       || (uCh3!=LINK_NOTUSED && uCh4==LINK_NOSTATE)
  	      )
  	   )break;
   }   
  	   
 
	/*for(j=0;(j<2048L*1024L)&&
		(  (    (READ_REGISTER_UCHAR(&(Adapter->pc2card_scbp->link_type[0]))!=LINK_NOTUSED)
		     && (READ_REGISTER_UCHAR(&(Adapter->pc2card_scbp->link_state[0]))==LINK_NOSTATE) 
		   )
		 ||(    (READ_REGISTER_UCHAR(&(Adapter->pc2card_scbp->link_type[1]))!=LINK_NOTUSED)
	         && (READ_REGISTER_UCHAR(&(Adapter->pc2card_scbp->link_state[1]))==LINK_NOSTATE)
	       )
	    );
	    j++)
	 {}
	 
	if((j==2048L*1024L)
		&&(((READ_REGISTER_UCHAR(&(Adapter->pc2card_scbp->link_type[0]))!=LINK_NOTUSED)
		&&(READ_REGISTER_UCHAR(&(Adapter->pc2card_scbp->link_state[0]))==LINK_NOSTATE))
		||((READ_REGISTER_UCHAR(&(Adapter->pc2card_scbp->link_type[1]))!=LINK_NOTUSED)
		&&(READ_REGISTER_UCHAR(&(Adapter->pc2card_scbp->link_state[1]))==LINK_NOSTATE))))
	{
	    
		return(1);
	}

     */
	 // replaced by fang
	 
    if( j==2048L*1024L)
    {
     NdisReadRegisterUchar((PUCHAR) &(Adapter->pc2card_scbp->link_type[0]),&uCh1);
  	 NdisReadRegisterUchar((PUCHAR) &(Adapter->pc2card_scbp->link_state[0]),&uCh2);
	 NdisReadRegisterUchar((PUCHAR) &(Adapter->pc2card_scbp->link_type[1]),&uCh3);
  	 NdisReadRegisterUchar((PUCHAR) &(Adapter->pc2card_scbp->link_state[1]),&uCh4);

     if(   (uCh1!=LINK_NOTUSED && uCh2==LINK_NOSTATE)
  	     || (uCh3!=LINK_NOTUSED && uCh4==LINK_NOSTATE)
  	   ) return(1);
  	}   

    for(k=0;k<NUMPORT;k++)
    {
     NdisReadRegisterUchar((PUCHAR) &(Adapter->pc2card_scbp->link_type[k]),&uCh1);
     if(   uCh1!=LINK_NOTUSED
		&& uCh1!=SDLC_3270 )
   	 {
   	   NdisWriteRegisterUchar( &(Adapter->pc2card_scbp->host_cmd[k]),LINKCMD_CONN);
	   NdisWriteRegisterUchar( &(Adapter->pc2card_scbp->local_rdy[k]),0);
     }
    }

	
    /*for(k=0;k<NUMPORT;k++){
		if((READ_REGISTER_UCHAR(&(Adapter->pc2card_scbp->link_type[k]))!=LINK_NOTUSED)
		&&(READ_REGISTER_UCHAR(&(Adapter->pc2card_scbp->link_type[k]))!=SDLC_3270))
		{
			WRITE_REGISTER_UCHAR(&(Adapter->pc2card_scbp->host_cmd[k]),LINKCMD_CONN);
			WRITE_REGISTER_UCHAR(&(Adapter->pc2card_scbp->local_rdy[k]),0);
		}
	}
	*/

	return(0);
}

int	i2sloadfsm(IN PNE2000_ADAPTER	Adapter,ULONG	cmd,PVOID	param)
{
	DBG_FUNC("i2sloadfsm")
	
	static	PUCHAR	startaddr;
	static	ULONG	length;
	ULONG	i,k;
	USHORT	st,cmp;
	
	


	switch(cmd){
	case T_RESET:
		// card reset
		i2sreset(Adapter);
		break;
	case T_SDLOAD:
		startaddr= Adapter->VirtualAddress;
		break;
	case T_DLOAD1:
		if(licopyin(param,(PUCHAR)(&length),sizeof(ULONG))==-1){
			DBG_ERROR(Adapter,("error sdlcioctl T_DLOAD1\n"));
			return(-1);
		}
		break;
	case T_DLOAD2:
		i2ssetwin(Adapter,
		          UMSEG,
                  (unsigned short) (startaddr-(PUCHAR)(Adapter->VirtualAddress)));             

        for( k=0;k<length;k++)
        {
         NdisWriteRegisterUchar(
           (PUCHAR) ( (PUCHAR)(Adapter->VirtualAddress)+(USHORT)(startaddr-(PUCHAR)(Adapter->VirtualAddress))%I2SWIN_SIZE+k),
           *((PUCHAR)param+k)            
           );
	    }
		/*WRITE_REGISTER_BUFFER_UCHAR(
		 	((PUCHAR)(Adapter->VirtualAddress)+(USHORT)(startaddr-(PUCHAR)(Adapter->VirtualAddress))%I2SWIN_SIZE),
			(PUCHAR)param,
			length);
			*/
		startaddr +=length;
		break;
	case T_EDLOAD:
		st=0;
		i2ssetwin(Adapter,UMSEG,0);
		i2sintr(Adapter);
		// Wait for minit run OK.     
		for(i=0; i<2048L*1024L; i++)
		{
		    NdisReadRegisterUshort(
		       (PUSHORT)(Adapter->VirtualAddress),
		       &cmp
		       );
				
			if(cmp != st){
                st=cmp;
				//st =READ_REGISTER_USHORT((PUSHORT)(Adapter->VirtualAddress));
				// by fang
				break;
			}
		}
		DBG_NOTICE(Adapter,("T_EDLOAD conf addr:%x\n",st));

		if(st>I2SWIN_SIZE){
			DBG_ERROR(Adapter,("conf addr too long:%x\n",st));
			return(-1);
		}
		if(licopyin(param,(PUCHAR)(&length),sizeof(ULONG))==-1){
			DBG_ERROR(Adapter,("licopyin config length"));
			return(-1);
		}
		if(length<=0){
			DBG_ERROR(Adapter,("err config len:%d\n",length));
			return(-1);
		}
		if((st+length)>I2SWIN_SIZE){
			DBG_ERROR(Adapter,("conf struct too long:addr%x:structlen:%x\n",st,length));
			return(-1);
		}

        for( k=0;k<length;k++)
        {
          NdisWriteRegisterUchar(
               (PUCHAR) ((PUCHAR)(Adapter->VirtualAddress)+st+k),
		       *((PUCHAR)((PUCHAR)param+sizeof(ULONG)+k)),
		       );
        }
		/*WRITE_REGISTER_BUFFER_UCHAR((PUCHAR)(Adapter->VirtualAddress)+st,
		(PUCHAR)param+sizeof(ULONG),
		length);
		*/
		if(pc2card_reset(Adapter,(PUCHAR)(Adapter->VirtualAddress),st)!=0){
			DBG_ERROR(Adapter,("pc2card error\n"));
			return(-1);
		} 
		break;
	}

	return(0);

}

int	i2sloadsoftware(IN PNE2000_ADAPTER       Adapter)
{
	UINT	olth;
	UINT	cnt,num;
	int	retval;
	DBG_FUNC("i2sloadsoftware")
	
	retval=i2sloadfsm(Adapter, T_SDLOAD,NULL);
	if(retval!=0){
			DBG_ERROR(Adapter,("T_SDLOAD\n"));
			return(retval);
	}
	olth=sizeof(softwarebuf);
	for(cnt=0; cnt<olth; cnt += num){
		num=(olth-cnt)<BUFSIZE?(olth-cnt):BUFSIZE;
		retval=i2sloadfsm(Adapter, T_DLOAD1,(PVOID)&num);
		if(retval!=0){
			DBG_ERROR(Adapter,("T_DLOAD1:%d\n",cnt));
			return(retval);
		}
		retval=i2sloadfsm(Adapter, T_DLOAD2,(PVOID)(softwarebuf+cnt));
		if(retval!=0){
			DBG_ERROR(Adapter,("T_DLOAD2:%d\n",cnt));
			return(retval);
		}
	}
	retval=i2sloadfsm(Adapter, T_EDLOAD,(PVOID)confbuf);
	if(retval!=0){
			DBG_ERROR(Adapter,("T_EDLOAD\n"));
			return(retval);
	}
	return(0);
}



BOOLEAN
CardReadyForTransmit(
    IN PNE2000_ADAPTER       Adapter
    )
{
	
	//NdisAcquireSpinLock(&Adapter->CardTransmitLock);
	if (Adapter->CardTransQue!=LAST_PKT)  {
	//	NdisReleaseSpinLock(&Adapter->CardTransmitLock);
		return FALSE;
	}	
	//The following is intended for ge
	//NdisReleaseSpinLock(&Adapter->CardTransmitLock);
	return TRUE;	
}




int
CardPrepareTransmit(
    IN PNE2000_ADAPTER       Adapter,
    IN USHORT                CardLine,
    IN PUCHAR				 buf_ptr,
    IN int                   BytesToSend
    )

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Functional Description:

    This routine will write the packet header information into the
    transmit buffer.  This assumes that the controller has notified the
    driver that the transmit buffer is empty.

Parameters:

    Adapter _ A pointer ot our adapter information structure.

    CardLine _ Specifies which line to use for the transmit (HTDSU_LINEx_ID).

    BytesToSend _ Number of bytes to transmit.

Return Values:

    None

---------------------------------------------------------------------------*/

{
    DBG_FUNC("CardPrepareTransmit")
	register    char    first;
	register    int    Tx_length;
	mblock_t    *mb_ptr;
	mb_list_t   *mlist_ptr;
	register    len_avl,retval;
	register    len_fact;
	register    infohead;
	int         realen=0;

	UCHAR       uch;
	USHORT      ush;
	ULONG       ulong;
	unsigned char  k;// by fang


    DBG_ENTER(Adapter);


    if (BytesToSend==0)
    	return  BytesToSend;
    ASSERT(BytesToSend > 0);
    ASSERT((CardLine==0) || (CardLine==1));

	//DbgPrint("acquire lock\n");
    NdisReadRegisterUchar( &(Adapter->pc2card_scbp->Txdata_rdy[CardLine]),&uch);
	if( uch ){
		DBG_ERROR(Adapter,("Txdata_rdy cardline:%d\n",CardLine));
		return I2SERR_BUFFULL;
	}

	NdisReadRegisterUchar( &(Adapter->pc2card_scbp->portmod[CardLine]),&uch);
	if(  (uch & MODUL_OFF)==MODUL_8)
	{	infohead=2;	}
  	else if( (uch & MODUL_OFF)==MODUL_128)
	{   infohead=3;	}

	NdisReadRegisterUshort( &(Adapter->pc2card_scbp->max_frame_len[CardLine]),&ush);
	if(BytesToSend>ush)
	{
		DBG_NOTICE(Adapter,("Txput BytesToSend:%d,max:%d\n",BytesToSend,Adapter->pc2card_scbp->max_frame_len[CardLine]));
		return I2SERR_LENOVER;
	}
	Tx_length=min(BytesToSend,ush);
	realen=Tx_length;
	first=1;
	mlist_ptr=get_mlistp(Adapter,Adapter->pc2card_scbp->Txdata_ptr[CardLine]);
	if(mlist_ptr==0){
		DBG_ERROR(Adapter,("err get_mlistp Txdata_put:%d\n",CardLine));
		return I2SERR_CARDERR;
	}

    NdisWriteRegisterUshort( &( ( (mb_list_t*)i2sf2w(Adapter,(PUCHAR)mlist_ptr) )->mb_list_count),
                    (USHORT)(Tx_length+infohead)
                      );
	//WRITE_REGISTER_USHORT(&(((mb_list_t*)i2sf2w(Adapter,(PUCHAR)mlist_ptr))->mb_list_count),(USHORT)(Tx_length+infohead));
	//replaced by fang
	mb_ptr=get_mbp(Adapter,((mb_list_t *)i2sf2w(Adapter,(PUCHAR)mlist_ptr))->mb_list_first);

	while (Tx_length != 0){
		mblock_t * real_mb_ptr;
		USHORT	offset;

		//current win is to mb_ptr, mb_ptr is flat value
		real_mb_ptr=(mblock_t *)i2sf2w(Adapter,(PUCHAR)mb_ptr);
		
		//WRITE_REGISTER_USHORT(&(real_mb_ptr->mb_offset),0);
		//replaced by fang as
		NdisWriteRegisterUshort( &(real_mb_ptr->mb_offset),0);

		if (first) {
			len_avl = FRMSIZ - infohead;        // 0x10b 
			real_mb_ptr->mb_flag |= SEND_XQ_FIRST;
			real_mb_ptr->mb_flag &= ~SEND_XQ_FINAL ;
			real_mb_ptr->mb_bufsize = infohead;
			first = 0;
		}
		else {
			len_avl = FRMSIZ;           // 0x10d 
			real_mb_ptr->mb_flag &= ~(SEND_XQ_FINAL | SEND_XQ_FIRST);                           // 0xfc == 0x3 
			real_mb_ptr->mb_bufsize = 0;
		}
		len_fact = min (len_avl, Tx_length);
		offset=real_mb_ptr->mb_moff+real_mb_ptr->mb_bufsize;
		//current win is to mb_ptr data, mb_ptr is flat value

		retval=i2ssetwin(Adapter,(USHORT)real_mb_ptr->mb_mseg,offset);
		if(retval!=0){
			DBG_ERROR(Adapter,("mbcopyout mbptr:\n"));
			return I2SERR_CARDERR;
		}

        for(k=0;k<len_fact;k++)
        { NdisWriteRegisterUchar( (PUCHAR)(Adapter->VirtualAddress)+offset%I2SWIN_SIZE+k,
                                  *(buf_ptr+k)
                                );
        }
		//WRITE_REGISTER_BUFFER_UCHAR((PUCHAR)(Adapter->VirtualAddress)+offset%I2SWIN_SIZE,buf_ptr,len_fact);

		real_mb_ptr=(mblock_t *)i2sf2w(Adapter,(PUCHAR)mb_ptr);
		real_mb_ptr->mb_bufsize += len_fact;
		mb_ptr=get_mbp(Adapter,real_mb_ptr->mb_next);
		buf_ptr += len_fact;
		BytesToSend -= len_fact;
		Tx_length -= len_fact;
	}   //end while 

	((mblock_t *)i2sf2w(Adapter,(PUCHAR)mb_ptr))->mb_flag |= SEND_XQ_FINAL;
	
	i2ssetwin(Adapter,(u_short)UMSEG,0);
	while(Adapter->pc2card_scbp->local_rdy[CardLine]==0)
	{}

	NdisWriteRegisterUchar(&(Adapter->pc2card_scbp->host_cmd[CardLine]),TXDATA_RDY);
	NdisWriteRegisterUchar(&(Adapter->pc2card_scbp->Txdata_rdy[CardLine]),1);
	NdisWriteRegisterUchar(&(Adapter->pc2card_scbp->local_rdy[CardLine]),0);

    DBG_LEAVE(Adapter);
    NdisStallExecution(1);
	return BytesToSend;
}	

VOID
HtDsuReceivePacket(
    IN PNE2000_ADAPTER   Adapter,
    IN USHORT	CardLine
    )

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Routine Description:

    This routine is called from HtDsuHandleInterrupt to handle a
    packet receive interrupt.  We enter here with interrupts enabled
    on the adapter and the processor, but the Adapter->Lock is held.

    We examine the adapter memory beginning where the adapter would have
    stored the next packet.
    As we find each good packet it is passed up to the protocol stack.
    This code assumes that the receive buffer holds a single packet.

    NOTE: The Adapter->Lock must be held before calling this routine.

Arguments:

    Adapter _ A pointer ot our adapter information structure.

Return Value:

    None.

---------------------------------------------------------------------------*/

{
    DBG_FUNC("HtDsuReceivePacket")

    NDIS_STATUS Status;

    /*
    // A pointer to our link information structure for the selected line 
device.
    */
  
    /*
    // How many bytes were received in this packet.
    */
    USHORT BytesReceived;

    /*
    // Non-zero if there were any errors detected in the packet.
    */
    USHORT RxErrors;

	mblock_t   *mb_ptr;
	mb_list_t  *mlist_ptr;

	register    final;
	register    new_frame;
	register    USHORT	len_avl,length;
	register    USHORT	offset,len_fact;
	register    infohead;
	register    realen=0;
	register		retval;
	PUCHAR	out_ptr;

	UCHAR   uch;
	USHORT  ush,ush2,k;//by fang
	
    DBG_ENTER(Adapter);

        /*
        // We have to copy the data to a system buffer so it can be
        // accessed one byte at a time by the protocols.
        // The adapter memory only supports word wide access.
        */
        NdisReadRegisterUchar(&(Adapter->pc2card_scbp->portmod[CardLine]),
                       &uch);
		
		if( ( uch & MODUL_OFF)==MODUL_8)
		{infohead=2;}
		else if( (uch & MODUL_OFF)==MODUL_128)
		{infohead=3;}

		mlist_ptr = get_mlistp(Adapter,Adapter->pc2card_scbp->Rxdata_ptr[CardLine]);
		//fang revised the get_mlistp
		if(mlist_ptr==0){
			DBG_ERROR(Adapter,("err get_mlistp Rxdata_get\n"));
			return;
		}
		mlist_ptr = (mb_list_t *) i2sf2w (Adapter , (PUCHAR)mlist_ptr);

		NdisReadRegisterUshort( &(mlist_ptr->mb_list_count),
		               &BytesReceived);
		
		//BytesReceived=READ_REGISTER_USHORT(&(mlist_ptr->mb_list_count));

		mb_ptr=(mblock_t *)get_mbp(Adapter,(u_short)(mlist_ptr->mb_list_first));
		//get_mbp will be modified by fang

		new_frame=1;
		final = 0;
		out_ptr=Adapter->TempBuf;
		length=sizeof(Adapter->TempBuf);
		while((length!=0)&&(mb_ptr!= NULL)&&(!final)){
			mblock_t  *real_mb_ptr;
	
			real_mb_ptr = (mblock_t *) i2sf2w(Adapter, (PUCHAR)mb_ptr);
			if (new_frame) {
			    NdisReadRegisterUshort(&(real_mb_ptr->mb_offset),&ush);
			    NdisWriteRegisterUshort(&(real_mb_ptr->mb_offset),
			               (USHORT)(ush+infohead)
			               );
				//WRITE_REGISTER_USHORT(&(real_mb_ptr->mb_offset),(USHORT)(READ_REGISTER_USHORT(&(real_mb_ptr->mb_offset))+infohead));
				new_frame = 0;
			}

            NdisReadRegisterUshort(&(real_mb_ptr->mb_bufsize),&ush);
            NdisReadRegisterUshort(&(real_mb_ptr->mb_offset),&ush2);
			if ( (len_avl = ush-ush2) == 0) {
				final = real_mb_ptr->mb_flag & RECV_RQ_FINAL;
				mb_ptr = get_mbp(Adapter,real_mb_ptr->mb_next);
			}
			else {
				len_fact = min(len_avl,length);
		
				offset=real_mb_ptr->mb_moff+real_mb_ptr->mb_offset;
				retval=i2ssetwin(Adapter,(u_short)(real_mb_ptr->mb_mseg),offset);
				if(retval!=0)
				{
			        DBG_ERROR(Adapter,("%din mbcopyout mbptr:\n"));			
			        return;
				}

                
                for(k=0;k<len_fact;k++)
                {
                 NdisReadRegisterUchar(
                   (PUCHAR)(Adapter->VirtualAddress)+offset%I2SWIN_SIZE+k,
    	            out_ptr+k
    	            );
   	            }
            	/*READ_REGISTER_BUFFER_UCHAR(
 	               (PUCHAR)(Adapter->VirtualAddress)+offset%I2SWIN_SIZE,
    	            out_ptr,
        	        len_fact
            	    );
            	 */
            	 //replaced by fang

				out_ptr += len_fact;
				realen=realen+len_fact;
				length-= len_fact;
				((mblock_t *)i2sf2w(Adapter,(PUCHAR)mb_ptr))->mb_offset  += len_fact;
			}
		}   //end while 
		i2ssetwin(Adapter,(u_short)UMSEG,0);

        NdisWriteRegisterUchar(&(Adapter->pc2card_scbp->Rxdata_rdy[CardLine]),0);

		//WRITE_REGISTER_UCHAR(&(Adapter->pc2card_scbp->Rxdata_rdy[CardLine]),0);
        while(1)
        {
         NdisReadRegisterUchar(&(Adapter->pc2card_scbp->lexp_rdy[CardLine]),&uch);
         if(uch)break;
        }
		//while(!READ_REGISTER_UCHAR(&(Adapter->pc2card_scbp->lexp_rdy[CardLine])))
		//{}

		NdisWriteRegisterUchar(&(Adapter->pc2card_scbp->hexp_cmd[CardLine]),RX_FINISH);
		NdisWriteRegisterUchar(&(Adapter->pc2card_scbp->lexp_rdy[CardLine]),0);
    	HtDsuHandlePacket(Adapter,Adapter->TempBuf,realen);
        DBG_LEAVE(Adapter);
}


NDIS_STATUS  CardStart(    IN PNE2000_ADAPTER       Adapter)
{
   // DBG_FUNC("Cardstart")
   

    NDIS_STATUS Status;
    USHORT st=0;
    USHORT  cmp;
    UINT	i;
	int	retval;

    //DBG_ENTER(Adapter);

    
    
    // CardIdentify
    // First we make sure the adapter is where we think it is.
    
  

	NdisRawWritePortUchar(Adapter->IoPAddr+WININT_R,(unsigned char)(WININT_RSTBIT&WININT_RST));
	i2ssetwin(Adapter,(unsigned short)UMSEG,0);
	NdisRawWritePortUchar(Adapter->IoPAddr+I2SADH_R,(unsigned char)(I2SRES_BIT&I2S_RESET)|(I2SMEM_BIT&I2SMEM_OPEN)|((NE2000_DPRAM(Adapter)>>AH_SHIFT)&I2SADH_BITS));

    //above no problem
	

	NdisWriteRegisterUshort(
                (PUSHORT)Adapter->VirtualAddress,
                0x5555);

	NdisReadRegisterUshort(
                (PUSHORT)Adapter->VirtualAddress,
                &st            
                );

	if(st!=0x5555){
        
        //DBG_ERROR(Adapter,("Adapter not found or invalid firmware:\n""write      = 0x5555 read = %X\n",st));

        Status = NDIS_STATUS_ADAPTER_NOT_FOUND;
        // Log error message and return.
        NdisWriteErrorLogEntry(
                Adapter->MiniportAdapterHandle,
                NDIS_ERROR_CODE_ADAPTER_NOT_FOUND,
                5,
				0x5555,
				st,
                Status,
                __FILEID__,
                __LINE__
                );

	    //DBG_LEAVE(Adapter);//omitted by fang
    	return (Status);
    	
	}

	NdisWriteRegisterUshort(
                (PUSHORT)Adapter->VirtualAddress,
                0xAAAA);

	NdisReadRegisterUshort(
                (PUSHORT)Adapter->VirtualAddress,
                &st
                );

	if(st!=0xAAAA){
        /*DBG_ERROR(Adapter,("Adapter not found or invalid firmware:\n"
                  "write      = 0xAAAA read = %X\n",
					st
				));
		*/

        Status = NDIS_STATUS_ADAPTER_NOT_FOUND;
        
        // Log error message and return.
        
        NdisWriteErrorLogEntry(
                Adapter->MiniportAdapterHandle,
                NDIS_ERROR_CODE_ADAPTER_NOT_FOUND,
                5,
				0xAAAA,
				st,
                Status,
                __FILEID__,
                __LINE__
                );

	   // DBG_LEAVE(Adapter);//omitted by fang
    	return (Status);
	}
   //above no problem
   
  // Reset the hardware to make sure we're in a known state.
  // Down load loadinit.  
	i2ssetwin(Adapter,(unsigned short)UMSEG,(unsigned short)I2SFLATOFF);
    /*WRITE_REGISTER_BUFFER_UCHAR(
                (PUCHAR)(Adapter->VirtualAddress)+I2SLDOFF-I2SFLATOFF,
                loadwin,I2SLDSIZ);
     */
     // deleted by fang for it is kernel call
	for(i=0; i<I2SLDSIZ; i++)
	{
        UCHAR  uCh;
	    NdisWriteRegisterUchar( (PUCHAR)(Adapter->VirtualAddress)+I2SLDOFF-I2SFLATOFF+i,
                         loadwin[i]
                         );
        NdisReadRegisterUchar(  (PUCHAR)(Adapter->VirtualAddress)+I2SLDOFF-I2SFLATOFF+i,
                         &uCh
                         );
 	 	    
		if(loadwin[i]!=uCh)
		{
		  /*DBG_ERROR(Adapter,("Adapter not found or invalid firmware:\n"
                  "I2S loadinit download fault! step%d\n win:%x mem:%x",
                  	i,
                  	loadwin[i],
                  	READ_REGISTER_UCHAR((PUCHAR)(Adapter->VirtualAddress)+I2SLDOFF-I2SFLATOFF+i)
				)
				);
		  */

          Status = NDIS_STATUS_ADAPTER_NOT_FOUND;
        
          // Log error message and return.
          NdisWriteErrorLogEntry(
                Adapter->MiniportAdapterHandle,
                NDIS_ERROR_CODE_ADAPTER_NOT_FOUND,
                5,
				i,
				0xAAAA,
				st,
                Status,
                __FILEID__,
                __LINE__
                );

	     // DBG_LEAVE(Adapter);
    	  return (Status);
		}
	}
	

	NdisWriteRegisterUshort(
                (PUSHORT) ( (PUCHAR)(Adapter->VirtualAddress)+I2SLDOFF - I2SFLATOFF + 1022),
                0x080C);


	i2ssetwin(Adapter,(unsigned short)UMSEG,0);
	// card work.   
	NdisRawWritePortUchar(Adapter->IoPAddr+I2SADH_R,
    (unsigned char)(I2SRES_BIT&I2S_WORK)|(I2SMEM_BIT&I2SMEM_OPEN)|((NE2000_DPRAM(Adapter)>>AH_SHIFT)&I2SADH_BITS));
	// Wait for loadinit run.       
	for(i=0; i<1024; i++);   

	st = 0;//USHORT
	// Wait for loadinit run OK.    
	for(i=0; i<1024L*2048L; i++){
		if(st == MEM_OK)break;
		
		NdisReadRegisterUshort(
		        (PUSHORT) Adapter->VirtualAddress,
		        &cmp
		        );
		if(st!=cmp)
		{
			//st = READ_REGISTER_USHORT(Adapter->VirtualAddress);
			//replaced by fang as
			st=cmp;

			switch(st){
			case MEM_OK ://DBG_PRINT(Adapter,("I2S Memory test OK!\n"));
				break;
			case LMOK :	//DBG_PRINT(Adapter,("I2S Lower Memory test OK!\n"));
				break;
			case MM0OK ://DBG_PRINT(Adapter,("I2S Mid-Range Memory 0 test OK!\n"));
				break;
			case MM1OK ://DBG_PRINT(Adapter,("I2S Mid-Range Memory 1 test OK!\n"));
				break;
			case MM2OK ://DBG_PRINT(Adapter,("I2S Mid-Range Memory 2 test OK!\n"));
				break;
			case UMOK ://DBG_PRINT(Adapter,("I2S Upper Memory test OK!\n"));
				break;
			case LMERR ://DBG_ERROR(Adapter,("I2S Lower Memory test error!\n"));
				Status = NDIS_STATUS_ADAPTER_NOT_FOUND;
   				//DBG_LEAVE(Adapter);
    			return (Status);
			case MM0ERR ://DBG_WARNING(Adapter,("I2S Memory0 test error!\n"));
				break;
			case MM1ERR ://DBG_WARNING(Adapter,("I2S Memory1 test error!\n"));
				break;
			case MM2ERR ://DBG_WARNING(Adapter,("I2S Memory2 test error!\n"));
				break;
			case UMERR ://DBG_ERROR(Adapter,("I2S Lower Memory test error!\n"));
				Status = NDIS_STATUS_ADAPTER_NOT_FOUND;
   				//DBG_LEAVE(Adapter);
    			return (Status);
 			default ://DBG_PRINT(Adapter,("I2S i=%d ST=0x%x\n",i,st));
				break;
			}
		i=0;
		} //end if
	}  //end for
	
	if((i >= 32000) || (st != MEM_OK)){
		//DBG_ERROR(Adapter,("I2S loadinit fault! i=%d ST=0x%x\n",i,st));
		Status = NDIS_STATUS_ADAPTER_NOT_FOUND;
   		//DBG_LEAVE(Adapter);

    	return (Status);
	
	}
    i2swininit(Adapter);

    retval=i2sloadsoftware(Adapter);
    if(retval!=0){
    	//DBG_ERROR(Adapter,("i2sloadsoftware err\n"));
    	Status = NDIS_STATUS_ADAPTER_NOT_FOUND;
    	//DBG_LEAVE(Adapter);

     	return (Status);
    }

    //DBG_LEAVE(Adapter);//omitted by fang

    

    return (NDIS_STATUS_SUCCESS);


}



//
// The 8930 chip set requires  4 bus clocks between chip selects
// 4 bus clocks at 8 MHZ is .5 uS, for a PPC running at 66 MHZ
// that works out to about 33 PPC instructions.  A 1 uS delay was
// added to gaurantee no chip selects within this window.
//


//comments by fang: 
//although it is defined in ne2000 for nt,and
//ne2000 can be used in win95,
//there maybe exist some problems in kernel
//calls such as WRITE_PORT_ULONG.

#define NE2000_WRITE_PORT_BUFFER_UCHAR(x, y, z) {                                \
    PUCHAR writeBuffer = y;                                               \
    ULONG writeCount;                                                     \
    for (writeCount = 0; writeCount < z; writeCount++, writeBuffer++) {   \
        *(volatile UCHAR * const)(x) = *writeBuffer;                      \
        KeFlushWriteBuffer();                                             \
        KeStallExecutionProcessor(1); \
    }                                                                     \
}

#define NE2000_WRITE_PORT_BUFFER_USHORT(x, y, z) {                               \
    PUSHORT writeBuffer = y;                                              \
    ULONG writeCount;                                                     \
    for (writeCount = 0; writeCount < z; writeCount++, writeBuffer++) {   \
        *(volatile USHORT * const)(x) = *writeBuffer;                     \
        KeFlushWriteBuffer();                                             \
        KeStallExecutionProcessor(1); \
    }                                                                     \
}

#define NE2000_WRITE_PORT_BUFFER_ULONG(x, y, z) {                                \
    PULONG writeBuffer = y;                                               \
    ULONG writeCount;                                                     \
    for (writeCount = 0; writeCount < z; writeCount++, writeBuffer++) {   \
        *(volatile ULONG * const)(x) = *writeBuffer;                      \
        KeFlushWriteBuffer();                                             \
        KeStallExecutionProcessor(1); \
    }                                                                     \
}

#define NE2000RawWritePortBufferUchar(Port,Buffer,Length) \
{ \
        ULONG _Port = (ULONG)(Port); \
        PUCHAR _Current = (Buffer); \
        PUCHAR _End = _Current + (Length); \
        for ( ; _Current < _End; ++_Current) { \
            WRITE_PORT_UCHAR((PUCHAR)_Port,*(UNALIGNED UCHAR *)_Current); \
            KeStallExecutionProcessor(1); \
        } \
}

#define NE2000RawWritePortBufferUshort(Port,Buffer,Length) \
{ \
        ULONG _Port = (ULONG)(Port); \
        PUSHORT _Current = (Buffer); \
        PUSHORT _End = _Current + (Length); \
        for ( ; _Current < _End; ++_Current) { \
            WRITE_PORT_USHORT((PUSHORT)_Port,*(UNALIGNED USHORT *)_Current); \
            KeStallExecutionProcessor(1); \
        } \
}

#define NE2000RawWritePortBufferUlong(Port,Buffer,Length) \
{ \
        ULONG _Port = (ULONG)(Port); \
        PULONG _Current = (Buffer); \
        PULONG _End = _Current + (Length); \
        for ( ; _Current < _End; ++_Current) { \
            WRITE_PORT_ULONG((PULONG)_Port,*(UNALIGNED ULONG *)_Current); \
            KeStallExecutionProcessor(1); \
        } \
}

BOOLEAN
CardSlotTest(
    IN PNE2000_ADAPTER Adapter
    );

/*BOOLEAN
CardRamTest(
    IN PNE2000_ADAPTER Adapter
    );
 */ 
 //omitted by fang 


#pragma NDIS_INIT_FUNCTION(CardCheckParameters)

BOOLEAN CardCheckParameters(
    IN PNE2000_ADAPTER Adapter
)

{
        return(FALSE);
   
}
#ifdef NE2000

#pragma NDIS_INIT_FUNCTION(CardSlotTest)


BOOLEAN CardSlotTest(
    IN PNE2000_ADAPTER Adapter
)

/*++

Routine Description:

    Checks if the card is in an 8 or 16 bit slot and sets a flag in the
    adapter structure.

Arguments:

    Adapter - pointer to the adapter block.

Return Value:

    TRUE, if all goes well, else FALSE.

--*/

{
    UCHAR Tmp;
    UCHAR RomCopy[32];
    UCHAR i;
	BOOLEAN found;

	USHORT k;// by fang for iteration

    //
    // Reset the chip
    //
    NdisRawReadPortUchar(Adapter->IoPAddr + NIC_RESET, &Tmp);
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RESET, 0xFF);

    //
    // Go to page 0 and stop
    //
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_COMMAND, CR_STOP | CR_NO_DMA);

    //
    // Pause
    //
    for(k=0;k<20;k++)
    NdisStallExecution(100);
    //NdisStallExecution(2000);
    //suggest by <for> repeation,each 100
    // by fang

    //
    // Check that it is stopped
    //
    NdisRawReadPortUchar(Adapter->IoPAddr + NIC_COMMAND, &Tmp);
    if (Tmp != (CR_NO_DMA | CR_STOP))
    {
        IF_LOUD(DbgPrint("Could not stop the card\n");)

        return(FALSE);
    }

    //
    // Setup to read from ROM
    //
    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_DATA_CONFIG,
        DCR_BYTE_WIDE | DCR_FIFO_8_BYTE | DCR_NORMAL
    );

    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_INTR_MASK, 0x0);

    //
    // Ack any interrupts that may be hanging around
    //
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_INTR_STATUS, 0xFF);

    //
    // Setup to read in the ROM, the address and byte count.
    //
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_ADDR_LSB, 0x0);

    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_ADDR_MSB, 0x0);

    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_LSB, 32);

    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_MSB, 0x0);

    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_COMMAND,
        CR_DMA_READ | CR_START
    );

    //
    // Read first 32 bytes in 16 bit mode
    //
    if (NE2000_ISA == Adapter->CardType)
    {
        for (i = 0; i < 32; i++)
        {
            NdisRawReadPortUchar(Adapter->IoPAddr + NIC_RACK_NIC, RomCopy + i);
        }
    }

    IF_VERY_LOUD( DbgPrint("Resetting the chip\n"); )

    //
    // Reset the chip
    //
    NdisRawReadPortUchar(Adapter->IoPAddr + NIC_RESET, &Tmp);
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RESET, 0xFF);

    //
    // Check ROM for 'B' (byte) or 'W' (word)
    // NOTE: If the buffer has bot BB and WW then use WW instead of BB
    IF_VERY_LOUD( DbgPrint("Checking slot type\n"); )

    if (NE2000_ISA == Adapter->CardType)
    {
	    found = FALSE;
        for (i = 16; i < 31; i++)
        {
            if (((RomCopy[i] == 'B') && (RomCopy[i+1] == 'B')) ||
                ((RomCopy[i] == 'W') && (RomCopy[i+1] == 'W'))
            )
            {
                if (RomCopy[i] == 'B')
                {
                    Adapter->EightBitSlot = TRUE;
	    			found = TRUE;
                }
                else
                {
                    Adapter->EightBitSlot = FALSE;
	    			found = TRUE;
	    			break;		// Go no farther
                }
            }
        }

        if (found)
	    {
	    	IF_VERY_LOUD( (Adapter->EightBitSlot?DbgPrint("8 bit slot\n"):
                                  DbgPrint("16 bit slot\n")); )
	    }
	    else
	    {
	    	//
	    	// If neither found -- then not an NE2000
	    	//
	    	IF_VERY_LOUD( DbgPrint("Failed slot type\n"); )
	    }
    }
    else
    {
        //
        //  PCMCIA adapters are always 16-bit.
        //
        Adapter->EightBitSlot = FALSE;
        found = TRUE;
    }

    return(found);
}

#endif // NE2000




//#pragma NDIS_INIT_FUNCTION(CardRamTest)

/*BOOLEAN
CardRamTest(
    IN PNE2000_ADAPTER Adapter
    )
 */

/*++

Routine Description:

    Finds out how much RAM the adapter has.  It starts at 1K and checks thru
    60K.  It will set Adapter->RamSize to the appropriate value iff this
    function returns TRUE.

Arguments:

    Adapter - pointer to the adapter block.

Return Value:

    TRUE, if all goes well, else FALSE.

--*/

/*{
    PUCHAR RamBase, RamPointer;
    PUCHAR RamEnd;

	UCHAR TestPattern[]={ 0xAA, 0x55, 0xFF, 0x00 };
	PULONG pTestPattern = (PULONG)TestPattern;
	UCHAR ReadPattern[4];
	PULONG pReadPattern = (PULONG)ReadPattern;

    for (RamBase = (PUCHAR)0x400; RamBase < (PUCHAR)0x10000; RamBase += 0x400) {

        //
        // Write Test pattern
        //

        if (!CardCopyDown(Adapter, RamBase, TestPattern, 4)) {

            continue;

        }

        //
        // Read pattern
        //

        if (!CardCopyUp(Adapter, ReadPattern, RamBase, 4)) {

            continue;

        }

        IF_VERY_LOUD( DbgPrint("Addr 0x%x: 0x%x, 0x%x, 0x%x, 0x%x\n",
                               RamBase,
                               ReadPattern[0],
                               ReadPattern[1],
                               ReadPattern[2],
                               ReadPattern[3]
                              );
                    )


        //
        // If they are the same, find the end
        //

        if (*pReadPattern == *pTestPattern) {

            for (RamEnd = RamBase; !((ULONG)RamEnd & 0xFFFF0000); RamEnd += 0x400) {

                //
                // Write test pattern
                //

                if (!CardCopyDown(Adapter, RamEnd, TestPattern, 4)) {

                    break;

                }

                //
                // Read pattern
                //

                if (!CardCopyUp(Adapter, ReadPattern, RamEnd, 4)) {

                    break;

                }

                if (*pReadPattern != *pTestPattern) {

                    break;

                }

            }

            break;

        }

    }

    IF_LOUD( DbgPrint("RamBase 0x%x, RamEnd 0x%x\n", RamBase, RamEnd); )

    //
    // If not found, error out
    //

    if ((RamBase >= (PUCHAR)0x10000) || (RamBase == RamEnd)) {

        return(FALSE);

    }

    //
    // Watch for boundary case when RamEnd is maximum value
    //

    if ((ULONG)RamEnd & 0xFFFF0000) {

        RamEnd -= 0x100;

    }

    //
    // Check all of ram
    //

    for (RamPointer = RamBase; RamPointer < RamEnd; RamPointer += 4) {

        //
        // Write test pattern
        //

        if (!CardCopyDown(Adapter, RamPointer, TestPattern, 4)) {

            return(FALSE);

        }

        //
        // Read pattern
        //

        if (!CardCopyUp(Adapter, ReadPattern, RamBase, 4)) {

            return(FALSE);

        }

        if (*pReadPattern != *pTestPattern) {

            return(FALSE);

        }

    }

    //
    // Store Results
    //

    Adapter->RamBase = RamBase;
    Adapter->RamSize = (ULONG)(RamEnd - RamBase);

    return(TRUE);

}
*/

#pragma NDIS_INIT_FUNCTION(CardInitialize)

BOOLEAN
CardInitialize(
    IN PNE2000_ADAPTER Adapter
    )

/*++

Routine Description:

    Initializes the card into a running state.

Arguments:

    Adapter - pointer to the adapter block.

Return Value:

    TRUE, if all goes well, else FALSE.

--*/

{
   return(TRUE);
}


#pragma NDIS_INIT_FUNCTION(CardReadEthernetAddress)

extern char HostIpAddress[];


BOOLEAN CardReadEthernetAddress(
    IN PNE2000_ADAPTER Adapter
)

/*++

Routine Description:

    Reads in the Ethernet address from the Novell 2000.

Arguments:

    Adapter - pointer to the adapter block.

Return Value:

    The address is stored in Adapter->PermanentAddress, and StationAddress if it
    is currently zero.

--*/

{
    

    UINT    c;
    UINT    i;


	// Assign the HostIpAddress to the Ethernet Address
	// Jimmy Guo
	for (i=0;i<6;i++)  
	Adapter->StationAddress[i]=Adapter->PermanentAddress[i]=((i<2)?0:HostIpAddress[i-2]);
	
    return(TRUE);
}

