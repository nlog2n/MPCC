#include <ndis.h>
#include "mpcccard.h"
#include "ne2000hw.h"//REFER TO MPCC
#include "ne2000sw.h"
#include "keywords.h"
#include "mpccfunc.h"
#include "typdef.h"
#include "const.h"
#include "extervar.h"

int
CardPrepareTransmit(
    IN PNE2000_ADAPTER       Adapter,
    IN USHORT               CardLine,     
	IN PUCHAR				buf_ptr,
    IN int               BytesToSend
    );


BOOLEAN
CardReadyForTransmit(
    IN PNE2000_ADAPTER        Adapter
    );

int StrComp(u_char *str1, u_char *str2, u_int length);

 /**************************************************************
			       FUN.C
		       function desciption & body

	**************************************************************/


void disable()
{}

void enable()
{}

/**********************************************************************
 NAME      : StrCopy

 DESCRIPTION:
		 Copy string from sour to dest.
		 The string length is gived in parameter length, not affected by
		 special characters such as '\0', '\n' etc.
 PARAMETERS:
		 (1)sour  : source string address
		 (2)dest  : destination string address
		 (3)length: string length
 RETURN VALUE:
		 None
**********************************************************************/

void StrCopy(u_char  *dest, u_char  *sour, u_int length)
{
  u_char  *str1,  *str2;

  if (sour!=NULL && dest!=NULL)   {
	  str1=sour;
	  str2=dest;
	  if(length==0)
	  {
		*str2='\0';
		return;
	  }
	  while (length--)
		 *str2++=*str1++;
	  *str2='\0';
  }
}




/*********************************************************
 NAME       : optcpy

 DESCRIPTION:
		 The function is to copy option string of destopt from string of souopt.
 PARAMETERS :
		 (1) souopt : the source option.
		 (2) desopt : the destination option.
 RETURN VALUE:
		 None
 *************************************************************/
void optcpy(struct Option  *desopt,struct Option  *souopt)
{
  if (desopt==NULL)
	  return;
  if (souopt==NULL)  {
	 desopt->Len=0;
	 desopt->Buf[0]='\0';
  }
  else  {
	 desopt->Len=souopt->Len;
	 StrCopy(desopt->Buf, souopt->Buf, souopt->Len);
  }
}




/*********************************************************
 NAME       : datacpy

 DESCRIPTION:
		 The function is to copy dataion string of destdata from string of soudata.
 PARAMETERS :
		 (1) soudata : the source data.
		 (2) desdata : the destination data.
 RETURN VALUE:
		 None
 *************************************************************/

void datacpy(struct UserData  *desud,struct UserData  *souud)
{
  if (desud==NULL)
	 return;
  if (souud==NULL)  {
	 desud->Len=0;
	 desud->Buf[0]='\0';
  }
  else   {
	 desud->Len=souud->Len;
	 StrCopy(desud->Buf, souud->Buf, souud->Len);
  }
}




/*********************************************************************
 NAME     : AddrCopy

 DESCRIPTION:
		 Copy address struct from sour to dest, including address length
		 and address itself.
		 The address string may include special characters such as '\0',
		 '\n' etc.
 PARAMETERS:
		 (1)dest: destination address
		 (2)sour: source address
 RETURN VALUE:
	None.
**********************************************************************/

 int AddrComp(struct NetAddr  *dest, struct NetAddr  *sour)
 {
	u_int len;

	if (dest->Len!=sour->Len)return 1;
	//len=(dest->Len/2)+(dest->Len%2);
	len= dest->Len;
	return (StrComp(dest->Addr, sour->Addr, len));
 }

 void AddrCopy(struct NetAddr  *dest, struct NetAddr  *sour)
 {
	u_int len;

	if (dest==NULL)
	  return;
	if (sour==NULL)  {
	  dest->Len=0;
	  dest->Addr[0]='\0';
	  return;
	}
	len=sour->Len;
	dest->Len=len;
	len=(len/2)+(len%2);
	StrCopy(dest->Addr, sour->Addr, len);
 }




/********************************************************************
 NAME      : StrComp

 DESCRIPTION:
		 Compare string str2 to str1.
		 The lengths of the two strings both are parameter length.
		 Both the two strings may include special characters such
		 as '\0', '\n' etc.
 PARAMETERS:
		 (1)str1  : string 1
		 (2)str2  : string 2
		 (3)length: string length
 RETURN VALUE:
		 (1) < 0 : str1 is less than str2
		 (2) ==0 : str1 is same as str2
		 (3) > 0 : str1 is greater than str2
		 << remarks: In fact, if the return value is not zero, the absolute
	  value of it is the position of first diffenent
	  character >>
**********************************************************************/
int StrComp(u_char *str1, u_char *str2, u_int length)
{
	u_char *pstr1, *pstr2;
	u_int len;

	pstr1=str1;
	pstr2=str2;
	len=length;
	while (len--)  {
	  if (*pstr1 < *pstr2) return(len-length);
	  else if (*pstr1++ > *pstr2++) return(length-len);
	}
	return(0);
}




/**********************************************************************
 NAME       : SearchFd

 DESCRIPTION:
		 Search for Fd in FdArray whose CallAddr is same as the parameter
		 addr (NOTES: LCN in the FdStruct must be FDUNCONNECTED, i.e. -3)
 PARAMETERS :
		 (1)addr: Called address structure
 RETURN VALUE:
		 (1) -1  : Not found right Fd
		 (2) >= 0: Fd whose CallAddr matchs the addr
***********************************************************************/

int SearchFd(void)
{
  int i=0, found=FALSE;

//  while (!found && i<=Adapter->init.HOC) {
	 if (FdArray[i].tcp_call==TCP_WAIT)
	   found=TRUE;
	 i++;
//  }
  if (found) return(i-1);
  else return(-1);
}

/*********************************************************************
 NAME       : CheckPktHead

 DESCRIPTION:
		 Check if a packet head is right
 PARAMETERS :
		 (1)lc  : logical channel pointer
		 (2)head: Packet head pointer need to be checked
 RETURN VALUE:
		 (1) >=0 : right
		 (2) < 0 : error packet head
		 << remarks: In fact, if the return value < 0, different value
	  signs different error. >>
***********************************************************************/

int CheckPktHead(struct LogicalChannel *lc, struct PktHead *head)
{
  if (head->Qbit != 0x0) return(ERR_QBIT);
  if (lc->Dbit != head->Dbit) return(ERR_DBIT);
  if (lc->Modul != head->Modul) return(ERR_MODUL);
  if (head->LCGN != lc->LCGN) return(ERR_LCGN);
  if (lc->LCN != head->LCN) return(ERR_LCN);
  return(1);
}





/**********************************************************************
 NAME       : GetNextWinNo

 DESCRIPTION:
		 Get the next sequence number in window
 PARAMETERS :
		 (1)modul: Module 8 or module 128
		 (2)num  : current sequence number
 RETURN VALUE:
		 (1)the next sequence number of parameter num
	  << under module parameter modul >>
***********************************************************************/

u_char GetNextWinNo(u_char modul, u_char num)
{
  u_char m;

  if (modul==0x10) m=8;
  else m=128;
  if (num < m-1)  return (num+1);
  else  return(0);
}




/*********************************************************************
 NAME       :GetWinSize

 DESCRIPTION:
		 get the window size and return it
 PARAMETERS:
		 (1)win: window pointer
 RETURN VALUE:
		 (1)the size of window win
 **********************************************************************/

 u_char GetWinSize(
 			IN PNE2000_ADAPTER    Adapter,
 			struct Window *win)
 {
	int i;

	i=win->High-win->Low;
	if (i >= 0)   return(i);
	else return (i+Adapter->init.WinSize);
 }




/*********************************************************************
 NAME       : IsWinFull

 DESCRIPTION:
		 check if a window is full
 PARAMETERS :
		 (1)win: window pointer
 RETURN VALUE:
		 (1)TRUE (==1): The window is full
		 (2)FALSE(==0): The window is not full
*********************************************************************/

int IsWinFull(	IN PNE2000_ADAPTER    Adapter,
				struct Window *win)
{
	return(win->Size==Adapter->init.WinSize);
}



/********************************************************************
  NAME       : BindPacket

  DESCRIPTION:
		 Bind a structured packet into a sturctureless string and
		 fill out the specifications in dest structure.
  PARAMETERS :
		 (1)sour: Source packet that will be packeted
		 (2)dest: Pcketed data
		<< RETURNED PARAMETER >>
  RETURN VALUR:
		 (1) >= 0: length of dest data
		 (2) <  0: bind failed
*********************************************************************/

int BindPacket(struct Packet *sour, struct PoolUnit *dest)
{
  u_int  len;   /* count dest Data length */
  u_int  k1, k2;
  u_char ch, *pstr;   /* dest string pointer */

  len=0;
  pstr=dest->Data;
  /*** bind packet head, excluding packet type identifier ***/
  /*** bind GFI ***/
  /*** bind Q bit ***/
  ch=sour->Head.Qbit << 7;
  /*** bind D bit ***/
  ch=ch | (sour->Head.Dbit << 6);
  /*** bind modulo bits ***/
  ch=ch | (sour->Head.Modul);
  /*** bind LCGN ***/
  ch=ch | (sour->Head.LCGN);
  *pstr++=ch;

  /*** bind LCN ***/
  *pstr++=sour->Head.LCN;

  switch(sour->Type) {
	 case CALL_REQ:
	 case CALL_ACPT:
		/*** call request packet and
		call accept packet ***/

		/*** bind packet type identifier ***/
		*pstr++=sour->Type;

		/*** bind calling and called DTE addresses length ***/
		k1=sour->Body.Call.CallAddr.Len;
		k2=sour->Body.Call.CalledAddr.Len;
		ch=((u_char)k1 << 4) | (u_char)k2;
		*pstr++=ch;

		/*** bind calling and called DTE address ***/
		k1=(k1/2)+(k1%2);
		k2=(k2/2)+(k2%2);
		len=k1+k2+4;
		StrCopy(pstr, sour->Body.Call.CalledAddr.Addr, k2);
		pstr+=k2;
		StrCopy(pstr, sour->Body.Call.CallAddr.Addr, k1);
		pstr+=k1;

		/*** bind facility length ***/
		*pstr++=(u_char)sour->Body.Call.Opt.Len;

		/*** bind facilities ***/
		k1=sour->Body.Call.Opt.Len;
		StrCopy(pstr, sour->Body.Call.Opt.Buf, k1);
		pstr+=k1;
		len+=(k1+1);

		/*** bind call user data ***/
		k1=sour->Body.Call.Data.Len;
		StrCopy(pstr, sour->Body.Call.Data.Buf, k1);
		pstr+=k1;
		len+=k1;
		break;

	 case DTE_DATA:
	 //The followng section is added by guo 
	 //Enable the poolunit to record the M_bit the the packet
	 //Begin section 
	 dest->Mbit=sour->Body.Data.Mbit;
		/*** DTE data packet ***/

		/*** bind P(R), M bit and P(S) ***/
		switch (sour->Head.Modul) {
		  case 0x10:
	  /*** 0x10==0001 0000, is modulo 8 ***/
	  ch=sour->Body.Data.PR << 5;
	  ch=ch | (sour->Body.Data.Mbit << 4);
	  ch=ch | (sour->Body.Data.PS << 1);
	  *pstr++=ch;
	  len=3;
	  break;
	case 0x20:
	  /*** 0x20==0010 0000, is modulo 128 ***/
	  ch=sour->Body.Data.PS << 1;
	  *pstr++=ch;
	  ch=sour->Body.Data.PR << 1;
	  ch=ch | sour->Body.Data.Mbit;
	  *pstr++=ch;
	  len=4;
	  break;
		}

		/*** bind user data ***/
		k1=sour->Body.Data.Data.Len;
		len+=k1;
		StrCopy(pstr, sour->Body.Data.Data.Buf, k1);
		pstr+=k1;
		break;

	 case DTE_RR:
	 case DTE_RNR:
	 case DTE_REJ:
		/*** DTE RR packet,
		DTE RNR packet and
		DTE REJ packet ***/

		/*** bind P(R) and packet type identifier ***/
		switch (sour->Head.Modul)  {
		  case 0x10:
			 /*** 0x10==0001 0000b, is modulo 8 ***/
			 ch=sour->Body.Flow.PR << 5;
			 ch=ch | sour->Type;
	  *pstr++=ch;
	  len=3;
	  break;
	case 0x20:
	  /*** 0x20==0010 0000b, is modulo 128 ***/
	  *pstr++=sour->Type;
	  ch=sour->Body.Flow.PR << 1;
	  *pstr++=ch;
	  len=4;
	  break;
		}
		break;

	 case DTE_INT:
		/*** DTE interrupt packet ***/

		/*** bind packet type identifier ***/
		*pstr++=DTE_INT;

		/*** bind interrupt user data ***/
		k1=sour->Body.Int.Data.Len;
      *pstr++=(u_char)k1;
      len=k1+3;
      StrCopy(pstr, sour->Body.Int.Data.Buf, k1);
      pstr+=k1;
      break;

    case DTE_INT_CONF:
      /*** DTE interrupt confirmation packet ***/

      /*** bind packet type identifier ***/
      *pstr++=DTE_INT_CONF;

      len=3;
      break;

    case REG_REQ:
      /*** registration request packet ***/

      /*** bind packet type identifier ***/
		*pstr++=REG_REQ;

      /*** bind DTE and DCE address length ***/
      k1=sour->Body.Regist.DteAddr.Len;
      k2=sour->Body.Regist.DceAddr.Len;
      ch=((u_char)k1 << 4) | (u_char)k2;
      *pstr++=ch;

      /*** bind DTE and DCE address ***/
      len=k1+k2+4;
      StrCopy(pstr, sour->Body.Regist.DceAddr.Addr, k2);
      pstr+=k2;
      StrCopy(pstr, sour->Body.Regist.DteAddr.Addr, k1);
      pstr+=k1;

      /*** bind registration length ***/
      k1=sour->Body.Regist.Reg.Len;
      *pstr++=(u_char)k1 & 0x7f;
      /* 0x7f==0111 1111 */

		/*** bind registration ***/
      StrCopy(pstr, sour->Body.Regist.Reg.Buf, k1);
      pstr+=k1;
      len+=(k1+1);
      break;

    case RESET_REQ:
      /*** reset request packet ***/

      /*** bind packet type identifier ***/
      *pstr++=RESET_REQ;

      /*** bind reseting cause ***/
      *pstr++=sour->Body.Reset.Cause;

      /*** bind diagnostic code ***/
      *pstr++=sour->Body.Reset.DiagCode;

      len=5;
      break;

    case DTE_RES_CONF:
      /*** DTE reset confirmation packet ***/

      /*** bind packet type identifier ***/
      *pstr++=DTE_RES_CONF;

      len=3;
      break;

    case RESTART_REQ:
      /*** restart request packet ***/

      /*** bind packet type identifier ***/
      *pstr++=RESTART_REQ;

      /*** bind restarting cause ***/
      *pstr++=sour->Body.Restart.Cause;

      /*** bind diagnostic code ***/
		*pstr++=sour->Body.Restart.DiagCode;

      len=5;
      break;

    case DTE_RST_CONF:
      /*** DTE restart confirmation packet ***/

      /*** bind packet type identifier ***/
      *pstr++=DTE_RST_CONF;

      len=3;
      break;

    case DIAG:
      /*** diagnostic packet ***/

      /*** bind packet type identifier ***/
      *pstr++=DIAG;

		/*** bind diagnostic code ***/
      *pstr++=sour->Body.Diagnose.DiagCode;

      /*** bind diagnostic explanation ***/
      StrCopy(pstr, sour->Body.Diagnose.Explan, HEADLEN);
      pstr+=HEADLEN;

      len=7;
      break;

	 case CLEAR_REQ:
		/*** clear request packet ***/
		/*** bind packet type identifier ***/
		*pstr++=CLEAR_REQ;

		/*** bind clearing cause ***/
		*pstr++=sour->Body.Clear.Cause;

		 /*** bind diagnostic code ***/
		 *pstr++=sour->Body.Clear.DiagCode;
		 len=5;

		 /*** bind calling and called DTE addresses length ***
		 k1=sour->Body.Clear.CallAddr.Len;
		 ch=(u_char)k1 << 4;
		 k1=(k1/2)+(k1%2);
		 k2=sour->Body.Clear.CalledAddr.Len;
		 ch=ch | (u_char)k2;
		 k2=(k2/2)+(k2%2);
		 *pstr++=ch;

		 *** bind calling and called DTE addresses ***
		 len=k1+k2+6;
		 StrCopy(pstr, sour->Body.Clear.CalledAddr.Addr, k2);
		 pstr+=k2;
		 StrCopy(pstr, sour->Body.Clear.CallAddr.Addr, k1);
		 pstr+=k1;

		 *** bind facility length ***
		 k1=sour->Body.Clear.Opt.Len;
		 *pstr++=(u_char)k1;

		 *** bind facilities ***
		 StrCopy(pstr, sour->Body.Clear.Opt.Buf, k1);
		 pstr+=k1;
		 len+=(k1+1);

		 *** bind clear user data ***
		 k1=sour->Body.Clear.Data.Len;
		 len+=k1;
		 StrCopy(pstr, sour->Body.Clear.Data.Buf, k1);
		 pstr+=k1;*/
		break;

	 case DTE_CLR_CONF:
		/*** DTE clear confirmation packet ***/
		/*** bind packet type identifier ***/
		*pstr++=DTE_CLR_CONF;

		len=3;
	 break;

	 default:
		return(ERR_PKT_TYP);
  }
  *pstr='\0';
  dest->Len=len;
  dest->Type=sour->Type;
  dest->LCN=sour->Head.LCN;
  return ((int)len);
}




/**********************************************************************
 NAME       : SendToLink

 DESCRIPTION:
       Send a packet to link level by calling link_level function
		 CardPrepareTransmit().
 PARAMETERS :
       (1)unit  : pool unit need to be transfered
 RETURN VALUE:
	(1) >=0 : data length
	(2) <0  : send fail
***********************************************************************/
void DupPoolUnit(struct PoolUnit *sour, struct PoolUnit *dest)
{
	dest->Type=sour->Type;
	dest->LCN=sour->LCN;
	dest->Mbit=sour->Mbit;
	dest->Len=sour->Len;
	StrCopy(dest->Data, sour->Data, sour->Len);
	return;
}


/*****************************************************************
	Function EnterX25CriticalRegion									 *
		Function to Wait until enter the x25 region              *
*****************************************************************/

VOID EnterX25CriticalRegion(
		IN PNE2000_ADAPTER    Adapter
							)
{
	KIRQL  OldIrql;
			
	/*  while (TRUE) {
	  	KeRaiseIrql(DISPATCH_LEVEL,&OldIrql);    
	    NdisAcquireSpinLock(&Adapter->X25LayerLock);*/
	  	if (!Adapter->InX25Layer)  {
	  	 Adapter->InX25Layer = TRUE;
	  	 //NdisReleaseSpinLock(&Adapter->X25LayerLock);
	  	 //KeLowerIrql(OldIrql);
		 //break;
	  	}
	  	//NdisReleaseSpinLock(&Adapter->X25LayerLock);
	  	else
	  	DbgPrint("Enter x25 region fail\n");
	  	//KeLowerIrql(OldIrql);
	// }
	
}


/*****************************************************************
	Function OutX25CriticalRegion 								 *
		Function to leave the x25 region                         *
*****************************************************************/

VOID OutX25CriticalRegion(
		IN PNE2000_ADAPTER    Adapter
							)
{
	KIRQL  OldIrql;
	
	/*KeRaiseIrql(DISPATCH_LEVEL,&OldIrql);    
	NdisAcquireSpinLock(&Adapter->X25LayerLock);*/
	Adapter->InX25Layer = FALSE;
	/*NdisReleaseSpinLock(&Adapter->X25LayerLock);
	KeLowerIrql(OldIrql);*/
	
}


/*****************************************************************
	Function EnterCardCriticalRegion							 *
		Function to Wait until enter the card send region        *
*****************************************************************/

VOID EnterCardCriticalRegion(
		IN PNE2000_ADAPTER    Adapter
							)
{
	  KIRQL  OldIrql;
	
	  	if (!Adapter->InQueue)  {
	  	 Adapter->InQueue = TRUE;
	  	}
	  	else
	    DbgPrint("Enter Card region fail\n");
	
}


/*****************************************************************
	Function OutCardCriticalRegion 								 *
		Function to leave the card region                        *
*****************************************************************/

VOID OutCardCriticalRegion(
		IN PNE2000_ADAPTER    Adapter
							)
{
	KIRQL  OldIrql;
	
	/*KeRaiseIrql(DISPATCH_LEVEL,&OldIrql);    

	NdisAcquireSpinLock(&Adapter->CardTransmitLock);*/
	Adapter->InQueue = FALSE;
	/*NdisReleaseSpinLock(&Adapter->CardTransmitLock);
	KeLowerIrql(OldIrql);*/

}

/*****************************************************************
	Function EnterRouterdCriticalRegion							 *
		Function to Wait until enter the Router region           *
*****************************************************************/

VOID EnterIpRouterCriticalRegion(
		IN PNE2000_ADAPTER    Adapter
							)
{
	  KIRQL  OldIrql;

	 /* while (TRUE) {
	    KeRaiseIrql(DISPATCH_LEVEL,&OldIrql);    

	    NdisAcquireSpinLock(&Adapter->IpRouterLock);*/
	  	if (!Adapter->InIpRouter)  {
	  	 Adapter->InIpRouter = TRUE;
	 //	 NdisReleaseSpinLock(&Adapter->IpRouterLock);
	 //	 KeLowerIrql(OldIrql);
	 //	 break;
	  	}
	  	else
	  //	NdisReleaseSpinLock(&Adapter->IpRouterLock);
	  	DbgPrint("Enter Ip Router region fail\n");
	  //  KeLowerIrql(OldIrql);

	// }
	
}


/*****************************************************************
	Function OutCardCriticalRegion 								 *
		Function to leave the Router region                         *
*****************************************************************/

VOID OutIpRouterCriticalRegion(
		IN PNE2000_ADAPTER    Adapter
							)
{
	KIRQL  OldIrql;
	
	/*KeRaiseIrql(DISPATCH_LEVEL,&OldIrql);    

	NdisAcquireSpinLock(&Adapter->IpRouterLock);*/
	Adapter->InIpRouter = FALSE;
	/*NdisReleaseSpinLock(&Adapter->IpRouterLock);
	KeLowerIrql(OldIrql);*/

}

/*****************************************************************
	Function VirtualTransmit 									 *
		Function to tranfer the x25 packet into the              *
	    Card transmit queue, it is a local buffer 				 * 
*****************************************************************/
int VirtualTransmit(
			   IN PNE2000_ADAPTER    Adapter,
			   IN struct PoolUnit *unit
)
{

	   int i;

	 //  DbgPrint("Virtual transmit\n");
	
	   EnterX25CriticalRegion(Adapter);
	   if (Adapter->SysFreeQue==LAST_PKT) {
		  Adapter->init.PktLevState=POOL_OVER;
		  OutX25CriticalRegion(Adapter);
		  return(POOL_OVER);
       }

       /*** delete a pool unit from system free queue ***/
       i=Adapter->SysFreeQue;
       ASSERT((0<=i)&&(i<POOLLEN));
       Adapter->SysFreeQue=Adapter->Pool[i].Next;
       OutX25CriticalRegion(Adapter);

       // Duplicate the packet now 
	   DupPoolUnit(unit,&Adapter->Pool[i]);

       /*** insert the unit into Card Transmit queue, */
       EnterCardCriticalRegion(Adapter);
	    if (Adapter->CardTransTail!=LAST_PKT) 
       	  Adapter->Pool[Adapter->CardTransTail].TransmitNext=i;
       Adapter->CardTransTail=i;
       if (Adapter->CardTransQue==LAST_PKT)
		  Adapter->CardTransQue=i;
	   Adapter->Pool[i].TransmitNext=LAST_PKT;
	   OutCardCriticalRegion(Adapter);
	  // DbgPrint("Out Virtual transmi tPacket head:%d Insert: %d\n ",Adapter->CardTransQue,i);
	   return i;
}


VOID TransmitQueue(
			  IN PNE2000_ADAPTER    Adapter
//			  IN BOOLEAN         InInterrupt
)
{
	 int i,ret;

	 KIRQL  OldIrql;
	


	//KeRaiseIrql(DISPATCH_LEVEL,&OldIrql);    
	//NdisAcquireSpinLock(&Adapter->QueueTransmitLock);
	if (!Adapter->InQueueTrans)
		Adapter->InQueueTrans = TRUE;
	else {
	//	NdisReleaseSpinLock(&Adapter->QueueTransmitLock);
	//	KeLowerIrql(OldIrql);
		DbgPrint("Enter transmitqueue fail \n");
		return ;
	}	
	//NdisReleaseSpinLock(&Adapter->QueueTransmitLock);
	//KeLowerIrql(OldIrql);

	 
	EnterCardCriticalRegion(Adapter);
	 
	 // No queued packet
	 if (Adapter->CardTransQue==LAST_PKT)  {
	 		OutCardCriticalRegion(Adapter);
	 	//	KeRaiseIrql(DISPATCH_LEVEL,&OldIrql);    
	 	//	NdisAcquireSpinLock(&Adapter->QueueTransmitLock);
			Adapter->InQueueTrans = FALSE;
		//	NdisReleaseSpinLock(&Adapter->QueueTransmitLock);
		//	KeLowerIrql(OldIrql);
		    return;
	 }

	 	
	 i= Adapter->CardTransQue;
	 // Transmit the queued X.25 packet
	 ret=CardPrepareTransmit(Adapter,0, Adapter->Pool[i].Data,Adapter->Pool[i].Len);
	 if (ret>=0)
	 {
	 	 // delete the packet from the queue
		 Adapter->CardTransQue=Adapter->Pool[i].TransmitNext;
		 if (Adapter->CardTransTail==i)
		 	Adapter->CardTransTail= LAST_PKT;
		 OutCardCriticalRegion(Adapter);
		 
		 // Release the buffer  used by the control frame
		 if (Adapter->Pool[i].Type!= DTE_DATA)  {
			EnterX25CriticalRegion(Adapter);
	 		Adapter->Pool[i].Next=Adapter->SysFreeQue;
			Adapter->SysFreeQue=i;
		    OutX25CriticalRegion(Adapter);
		}
		
 	 // Release the buffer  used by the control frame
	 //	DbgPrint("Queue Transmit packet %d  head %d \n",i,Adapter->CardTransQue); 
	 }
	 else{
	 	OutCardCriticalRegion(Adapter);
		DbgPrint("Transmit err:%d\n",ret);
	 }

	 Adapter->InQueueTrans = FALSE;
	 

}



int SendToLink(
			   IN PNE2000_ADAPTER    Adapter,
			   IN struct PoolUnit *unit
)
{
	 int ret,i;
	 struct LogicalChannel *LC;
	 BOOLEAN  VirtualTran= FALSE;
	 

   ASSERT((0<=unit->LCN)&&(unit->LCN<VC_NUM));
   LC=&Adapter->Channel[unit->LCN];

   
   if (CardReadyForTransmit(Adapter))  {
      		ret=CardPrepareTransmit(Adapter,0, unit->Data, unit->Len);
  			// Send to card fail ,then virtual transmit.   		
			if (ret<0)  {
			//  DbgPrint("SendtoLink fail:%d\n",ret);
			  if (unit->Type!=CALL_REQ) {
			  	VirtualTran = TRUE;
	   		  	ret=VirtualTransmit(Adapter,unit);
	   		  }
	   		  else {	
	   		  			//NdisStallExecution(10);
	   		  	   		return ret;
	   		  	   	}	  
   			}
   		
	}		
   else {
   		  VirtualTran = TRUE;
   		  ret=VirtualTransmit(Adapter,unit);
   		  //TransmitQueue(Adapter);
   		  //TransmitQueue(Adapter);
   		}

   if (ret <0)
   	return -1;
   	
   switch(unit->Type)  {
     case DTE_DATA:
       /*** data packet. insert into send queue ***/
        EnterX25CriticalRegion(Adapter);
        if (!VirtualTran)  { 
		   // Not virtual transmit
	       if (Adapter->SysFreeQue==LAST_PKT) {
			  Adapter->init.PktLevState=POOL_OVER;
        	  OutX25CriticalRegion(Adapter);
			  return(POOL_OVER);
    	   }
	       /*** delete a pool unit from system free queue ***/
    	   i=Adapter->SysFreeQue;
	       ASSERT((0<=i)&&(i<POOLLEN));
    	   Adapter->SysFreeQue=Adapter->Pool[i].Next;
           ASSERT((0<=i)&&(i<POOLLEN));
           OutX25CriticalRegion(Adapter);
           DupPoolUnit(unit,&Adapter->Pool[i]);
	       EnterX25CriticalRegion(Adapter);
           
        }
	   else i= ret;
	   
       /*** insert the unit into local send queue,
	    waiting for confirmation. ***/
       if (LC->SendTail != LAST_PKT)
		  Adapter->Pool[LC->SendTail].Next=i;
       LC->SendTail=i;
       if (LC->SendQue==LAST_PKT)
		  LC->SendQue=i;
	   Adapter->Pool[i].Next=LAST_PKT;
	   LC->SendNo=GetNextWinNo(LC->Modul, LC->SendWin.High);
	   LC->SendWin.High=LC->SendNo;
	   LC->SendWin.Size++;
	   // Added by guo 
	   // Set the timer to record the last packet
 	   // Begin of section
        OutX25CriticalRegion(Adapter);
	   //NdisMCancelTimer(&LC->CallTimer,&result);
	   //NdisMSetTimer(&LC->CallTimer,DATA_PACKET_TIMEOUT);
	  // End of section 
	  break;
   case CALL_REQ:
		switch(LC->State)  {
		 case DCE_WAIT:
		 	 LC->State=CALL_COLI;
			 break;
		 case READY:
			 LC->State=DTE_WAIT;
		     break;
		 case CLR_IND:
		     break;
	     }
     break;
   case CALL_ACPT:
    	switch(LC->State)  {
		 case DCE_WAIT:
		 case CALL_COLI:
		   LC->State=DATA_TRANS;
		 break;
		case CLR_IND:
		 break;
		} 
      break;

   case RESET_REQ:
		switch(LC->State) {
		case FLOW_CTL:
		  LC->State=RES_REQ;
		  break;
		case RES_REQ:
			 break;
		case RES_IND:
		  LC->State=FLOW_CTL;
		  break;
       }
       break;
   case DTE_RES_CONF:
		switch(LC->State)  {
		case RES_IND:
		  LC->State=FLOW_CTL;
		break;
		}
       break;

   case RESTART_REQ:
		switch(LC->State)  {
		case READY:
			LC->State=RST_REQ;
			break;
		case RST_IND:
			  LC->State=READY;
			break;
		case RST_REQ:
			break;
		default:	
			LC->State=RST_REQ;
	       }
       break;

   case DTE_RST_CONF:
		switch(LC->State)  {
		case RST_IND:
		  LC->State=READY;
		  break;
		}
	   break;

   case CLEAR_REQ:
	   switch(LC->State)  {
	   case CLR_REQ:
			break;
		// Added by Guo
		// Clear Call_req.
	   case DCE_WAIT:	
	   case CLR_IND:
  	   case CALL_COLI:
		    LC->State=READY;
			break;
	   case READY:
		// Clear a ready channel .
			break;
       default:
		    LC->State=CLR_REQ;
		    break;
       }
       break;

   case DTE_CLR_CONF:
	switch(LC->State)  {
	case CLR_IND:
		  LC->State=READY;
		break;
		}
     break;
   }	
	return ret;
}

int OldSendToLink(IN PNE2000_ADAPTER    Adapter,
			   IN struct PoolUnit *unit)
{
  int i;
  struct LogicalChannel *LC;
  BOOLEAN  result;


 //  NdisAcquireSpinLock(&Adapter->X25LayerLock);

   //NdisAcquireSpinLock(&Adapter->Lock);
   ASSERT((0<=unit->LCN)&&(unit->LCN<VC_NUM));

   LC=&Adapter->Channel[unit->LCN];
   switch(unit->Type)  {
     case DTE_DATA:
       /*** data packet. insert into send queue ***/
	   EnterX25CriticalRegion(Adapter);

       if (Adapter->SysFreeQue==LAST_PKT) {
		  Adapter->init.PktLevState=POOL_OVER;
	      OutX25CriticalRegion(Adapter);
		  return(POOL_OVER);
       }

       /*** delete a pool unit from system free queue ***/
       i=Adapter->SysFreeQue;
       ASSERT((0<=i)&&(i<POOLLEN));
       Adapter->SysFreeQue=Adapter->Pool[i].Next;

       /*** insert the unit into local send queue,
	    waiting for confirmation. ***/
       if (LC->SendTail != LAST_PKT)
		  Adapter->Pool[LC->SendTail].Next=i;
       LC->SendTail=i;
        if (LC->SendQue==LAST_PKT)
		  LC->SendQue=i;
	   Adapter->Pool[i].Next=LAST_PKT;
       OutX25CriticalRegion(Adapter);
		
       ASSERT((0<=i)&&(i<POOLLEN));
       
       DupPoolUnit(unit,&Adapter->Pool[i]);
       i=CardPrepareTransmit(Adapter,0, unit->Data, unit->Len);
       if (i >= 0)  
       {
 		  EnterX25CriticalRegion(Adapter);
 		  LC->SendNo=GetNextWinNo(LC->Modul, LC->SendWin.High);
		  LC->SendWin.High=LC->SendNo;
		  LC->SendWin.Size++;
		  // Added by guo 
		  // Set the timer to record the last packet
		  // Begin of section
        	OutX25CriticalRegion(Adapter);
		  //NdisMCancelTimer(&LC->CallTimer,&result);
		  //NdisMSetTimer(&LC->CallTimer,DATA_PACKET_TIMEOUT);
		  // End of section 
	   }
		 break;

     case CALL_REQ:
		i=CardPrepareTransmit(Adapter,0, unit->Data, unit->Len);       if (i >= 0) {
		switch(LC->State)  {
		 case DCE_WAIT:
		 LC->State=CALL_COLI;
			 break;
		 case READY:
		 LC->State=DTE_WAIT;
		     break;
		 case CLR_IND:
		     break;
	 }
     }
     break;

     case CALL_ACPT:
       i=CardPrepareTransmit(Adapter,0, unit->Data, unit->Len);
       if (i >= 0)  {
		switch(LC->State)  {
		 case DCE_WAIT:
		 case CALL_COLI:
		   LC->State=DATA_TRANS;
		 break;
		}
		case CLR_IND:
		 break;
       }
	  break;

     case RESET_REQ:
       i=CardPrepareTransmit(Adapter,0, unit->Data, unit->Len);
       if (i >= 0) {
		switch(LC->State) {
		case FLOW_CTL:
		  LC->State=RES_REQ;
		  break;
		case RES_REQ:
			 break;
		case RES_IND:
		  LC->State=FLOW_CTL;
		  break;
		}
       }
       break;
     case DTE_RES_CONF:
       i=CardPrepareTransmit(Adapter,0, unit->Data, unit->Len); 
       if (i >= 0)  {
		switch(LC->State)  {
		case RES_IND:
		  LC->State=FLOW_CTL;
		break;
		}
       }
       break;

     case RESTART_REQ:
	     i=CardPrepareTransmit(Adapter,0, unit->Data, unit->Len);
		 if (i >= 0)  {
			switch(LC->State)  {
			case READY:
			  LC->State=RST_REQ;
			break;
			case RST_IND:
			  LC->State=READY;
			break;
			case RST_REQ:
			break;
			}
	       }
       break;

     case DTE_RST_CONF:
       i=CardPrepareTransmit(Adapter,0, unit->Data, unit->Len);
	   if (i >= 0)  {
		switch(LC->State)  {
		case RST_IND:
		  LC->State=READY;
		  break;
		}
	   }
	   break;

     case CLEAR_REQ:
       i=CardPrepareTransmit(Adapter,0, unit->Data, unit->Len);      
       if (i >= 0)  {
	   switch(LC->State)  {
		case CLR_REQ:
			break;
		// Added by Guo
		// Clear Call_req.
		case DCE_WAIT:	
		case CLR_IND:
		case CALL_COLI:
		    LC->State=READY;
			break;
		case READY:
		// Clear a ready channel .
			break;
		default:
		    LC->State=CLR_REQ;
		    break;
		}
       }
       break;

     case DTE_CLR_CONF:
       i=CardPrepareTransmit(Adapter,0, unit->Data, unit->Len);      
       if (i >= 0)  {
		switch(LC->State)  {
		case CLR_IND:
		  LC->State=READY;
		break;
		}
       }
      break;

     default:
	       i=CardPrepareTransmit(Adapter,0, unit->Data, unit->Len);
      break;
   }

  if(i < 0) {
//    Adapter->init.PktLevState=LLI_FAIL;
//    NdisReleaseSpinLock(&Adapter->X25LayerLock);
    DbgPrint("CardPrepareTransmit err:%d\n",i);
    return LLI_FAIL;
  }

//  NdisReleaseSpinLock(&Adapter->X25LayerLock);
	
  return i;
}


/**********************************************************************
 NAME       : SendFlowCtlPkt

 DESCRIPTION:
       Send a flow control packet
 PARAMETERS :
       (2)lc  : Logical channel pointer
       (1)type: packet type identifier(RR, RNR or REJ)
 RETURN VALUE:
       (1) >=0 : success
       (2) <0  : fail
**********************************************************************/

int  SendFlowCtlPkt(IN PNE2000_ADAPTER    Adapter,
					IN struct LogicalChannel *lc,
					IN u_char type)
{
  int i;
  u_char ch, *pstr;
  struct PoolUnit unit;


  UCHAR  tmpchar;

  pstr=unit.Data;
  /*** bind GFI ***/
  ch=lc->Modul;

  tmpchar = (ch | (lc->LCGN));	
  *pstr++ = tmpchar ;
  tmpchar = (lc->LCN);
  *pstr++=  tmpchar ;

  EnterX25CriticalRegion(Adapter);
      		
  switch (lc->Modul) {
    case 0x10:
      /*** 0x10==0001 0000, is module 8 ***/
      tmpchar = (lc->RecvNo << 5);
	  ch= tmpchar ;
	  tmpchar = (ch | type) ;
      *pstr++=  tmpchar;
      unit.Len=3;
      break;
    case 0x20:
		/*** 0x20==0010 0000b, is module 128 ***/
      *pstr++=type;
      tmpchar = (lc->RecvNo << 1);
	  *pstr++= tmpchar ;
      unit.Len=4;
      break;
  }

  OutX25CriticalRegion(Adapter);
  	
  unit.Type=type;
  unit.LCN= lc->LCN ;

  i=SendToLink(Adapter,&unit);
  return 0;
}





/*********************************************************************
 NAME       : SendIntConfPkt

 DESCRIPTION:
       Send interrupt confirmation packet to
       the very logical channel
 PARAMETERS :
       (1)lcn : Logical channel number
 RETURN VALUE:
       (1) >=0 : success
       (2) <0  : fail
 ********************************************************************/

int  SendIntConfPkt(IN PNE2000_ADAPTER    Adapter,
					IN u_char lcn)
{
  int i;
  u_char ch, *pstr;
  struct LogicalChannel *lc;
  struct PoolUnit unit;


  pstr=unit.Data;

  ASSERT((0<=lcn)&&(lcn<VC_NUM));
  lc=&Adapter->Channel[lcn];
  ch=lc->Modul;
  *pstr++=(ch | lc->LCGN);
  *pstr++=lcn;
  *pstr++=DTE_INT_CONF;
  *pstr='\0';
  unit.Type=DTE_INT_CONF;
  unit.LCN=lcn;
  unit.Len=3;

  i=SendToLink(Adapter,&unit);
  return(i);
}




/*********************************************************************
 NAME       : SendResetPkt

 DESCRIPTION:
		 Send a reset_request packet
       or send a reset_confirmation packet
 PARAMETERS :
       (1)lcn  : logical channel number
       (2)type : Packet type
		 (reset_request packet or reset_confirmation packet )
       (3)cause: reset cause
       (4)code : diagnostic code
 RETURN VALUE:
       (1) >=0 : success
       (2) <0  : fail
**********************************************************************/

int  SendResetPkt(IN PNE2000_ADAPTER    Adapter,
				IN u_char lcn,IN  u_char type,
				IN u_char cause,IN u_char code)
{
   int i;
   struct PoolUnit unit;
   struct LogicalChannel *lc;
   u_char ch, *pstr;

   ASSERT((0<=lcn)&&(lcn<VC_NUM));
   lc=&Adapter->Channel[lcn];
   pstr=unit.Data;
   unit.Type=type;
   /*** bind Dbit ***/
   ch=lc->Dbit << 6;
   /*** bind Module ***/
   ch=ch | lc->Modul;
   /*** LCGN==0000 ***/
   *pstr++=ch;

   /*** bind LCN ***/
   *pstr++=lcn;
   switch (type)  {
     case RESET_REQ:
       /*** send a reset_request packet ***/
       /*** bind packet type identifier ***/
       *pstr++=RESET_REQ;
       *pstr++=cause;
       *pstr++=code;
		 *pstr='\0';
       unit.LCN=lcn;
       unit.Len=5;
       i=SendToLink(Adapter,&unit);
		 break;
	  case DTE_RES_CONF:
		 /*** bind packet type identifier ***/
		 /*** send a resetr_confirmation packet ***/
		 *pstr++=DTE_RES_CONF;
		 *pstr='\0';
		 unit.LCN=lcn;
		 unit.Len=3;
		 i=SendToLink(Adapter,&unit);
		 break;
	  default:
#ifdef _DEBUG
	d_prn("err 2040\n");
#endif
		 i=ERR_PKT_TYP;
		 break;
	}
	return(i);
}





/*******************************************************************
 NAME       : SendRestartPkt

 DESCRIPTION:
       Send a restart request or restart confirmation packet to
       the very logical channel
 PARAMETERS :
       (1)type : packet type
		 (restart request or restart confirmation packet)
       (2)cause: restart cause
       (3)code : diagnostic code
 RETURN VALUE:
		 (1) >=0 : success
       (2) <0  : fail
********************************************************************/

int  SendRestartPkt(IN PNE2000_ADAPTER    Adapter,
					IN u_char type, 
					IN u_char cause, 
					IN u_char code)
{
  int i;
  struct PoolUnit unit;
  u_char *pstr;

  unit.Type=type;
  unit.LCN='\0';;
  pstr=unit.Data;
  /*** bind GFI, LCGN and LCN ***/
  switch(type)   {
    case RESTART_REQ:
       *pstr++=0x10;
       *pstr++='\0';
       *pstr++=RESTART_REQ;
       *pstr++=cause;
	   *pstr++=code;
       unit.Len=5;
       *pstr='\0';
       disable();
       i=SendToLink(Adapter,&unit);
       enable();
       break;
    case DTE_RST_CONF:
       *pstr++=0x10;
       *pstr++='\0';
       *pstr++=DTE_RST_CONF;
       unit.Len=3;
       *pstr='\0';
       i=SendToLink(Adapter,&unit);
       break;
  }


  if(type==RESTART_REQ)
  {
	T20=Adapter->init.T20;                        /* set T20 timer */
    TimeOut=FALSE;
	Adapter->Channel[0].State=RST_REQ;          /* set State of CLN */
	Adapter->init.PktLevState=RESTARTING;
  }
  return(i);
}

int QueuedPacket(
			IN PNE2000_ADAPTER    Adapter,
			IN  struct LogicalChannel *lc)
{
	int packetnum = 1;
	int queuenum  = 0;

	queuenum = lc->RecvQue;

	if (lc->RecvQue==LAST_PKT)
	 	return 0;
	while (queuenum != lc->RecvTail)
	{
		packetnum++;
		ASSERT((0<=queuenum)&&(queuenum<POOLLEN));
		queuenum = Adapter->Pool[queuenum].Next;
	}

	return packetnum;
}



/*********************************************************************
 NAME       : RecvPacket

 DESCRIPTION:
		 Receive a packet whose data is in buf and insert the
		 packet into receive queue
 PARAMETERS :
		 (1)lc     : The very logical channel pointer
		 (2)buf    : Packet data buffer
		 (3)length : data length
		 (4)type   : packet type
 RETURN VALUE:
		 (1) < 0 : Fail to receive
		 (2) >=0 : Index of chosed buffer pool unit
**********************************************************************/

int RecvPacket(
			IN PNE2000_ADAPTER    Adapter,
			struct LogicalChannel *lc, u_char *buf,
			 u_int length, u_char type)
{
  int   i;

  EnterX25CriticalRegion(Adapter);
  if (Adapter->SysFreeQue==LAST_PKT) {
	  Adapter->init.PktLevState=POOL_OVER;
      OutX25CriticalRegion(Adapter);
	  return POOL_OVER;
  }

  /*disable();*/
  /*** choose a free pool unit from system free queue ***/
  i=Adapter->SysFreeQue;
  ASSERT((0<=i)&&(i<POOLLEN));
  Adapter->SysFreeQue=Adapter->Pool[i].Next;

  OutX25CriticalRegion(Adapter);


  /*** load data from buf into the chosed unit ***/
  Adapter->Pool[i].Len=length;
  StrCopy(Adapter->Pool[i].Data, buf, length);
  Adapter->Pool[i].Type=type;
  Adapter->Pool[i].LCN=lc->LCN;

  EnterX25CriticalRegion(Adapter);
	
  /*** insert the pool unit into local receive queue ***/
  switch (type) {
	 case DCE_INT:
		/*** expedited data, insert into queue head ***/
		if (lc->RecvQue==LAST_PKT)  {
	 /*** receive queue is empty ***/
	 lc->RecvTail=lc->RecvQue=i;
     ASSERT((0<=i)&&(i<POOLLEN));
	 Adapter->Pool[i].Next=LAST_PKT;
		}
		else   {
	 /*** receive queue isn't empty ***/
     ASSERT((0<=i)&&(i<POOLLEN));
	 Adapter->Pool[i].Next=lc->RecvQue;
	 lc->RecvQue=i;
		}
		break;
	 default:
		/*** insert into queue tail ***/
		if (lc->RecvQue==LAST_PKT)  {
		 /*** receive queue is empty ***/
		 lc->RecvTail=lc->RecvQue=i;
		 Adapter->Pool[i].Next=LAST_PKT;
		}
		else   {
		 /*** receive queue isn't empty ***/
		 Adapter->Pool[lc->RecvTail].Next=i;
		 Adapter->Pool[i].Next=LAST_PKT;
		 lc->RecvTail=i;
		}
//		DbgPrint("Received packet %d \n",QueuedPacket(Adapter,lc));		
		break;
  }

  OutX25CriticalRegion(Adapter);

  /*enable();*/
  return i;
}




/**********************************************************************
 NAME       : AssemblePacket

 DESCRIPTION:
       Assemble packets from receivce queue of the very logical
       channel LCN, which corrects the queues at the same time
 PARAMETERS :
       (1)lc     : logical channel pointer
       (1)buf    : Data buffer(pointer)
       (2)length : Data maximum length
       (3)flags  : data specification
 RETURN VALUE:
       (1) >=0 : recveived data length
		 (2) < 0 : assemble fail
**********************************************************************/

int  AssemblePacket(
			IN PNE2000_ADAPTER    Adapter,
			 struct LogicalChannel *lc, u_char  *buf,
			 u_int length, char flags)
{
  u_int RecvLen=0, i;
  int k;
  u_char   *dest;
  struct PoolUnit *unit, *pre;

  EnterX25CriticalRegion(Adapter);
 
  if (lc->RecvQue==LAST_PKT)  {
      OutX25CriticalRegion(Adapter);
	  return QUE_EMPTY;
  }

  /*disable();*/
  ASSERT((0<=lc->RecvQue)&&(lc->RecvQue<POOLLEN));

  unit=&Adapter->Pool[lc->RecvQue];
  
  
  switch(flags) {
	 case T_EXPEDITED:
		 k=lc->RecvQue;
		 /*** delete the pool unit from local receive queue ***/
		lc->RecvQue=unit->Next;
		if (lc->RecvQue==LAST_PKT)
		 lc->RecvTail=LAST_PKT;

		/*** insert the pool unit into system free queue ***/
		unit->Next=Adapter->SysFreeQue;
		Adapter->SysFreeQue=k;
        OutX25CriticalRegion(Adapter);
		break;
	 case T_NORMAL:
        OutX25CriticalRegion(Adapter);
		dest=buf;
		do  {
		 i=unit->Len;
		 RecvLen+=i;
		 if (RecvLen > length)  {
		 /*** ERROR: data overflow ***/
		 #ifdef _DEBUG
			 d_prn("err 2060\n");
		 #endif
			 /*enable();*/
			 return DATA_OVER;
		 }
		 StrCopy(dest, unit->Data, i);
		 dest+=i;
		 pre=unit;
		// ASSERT((0<=unit->Next)&&(unit->Next<POOLLEN));
		 if (unit->Next!= LAST_PKT)		
			 unit=&Adapter->Pool[unit->Next];
		}while (pre->Mbit && pre->Next!=LAST_PKT);

		/*** insert the assembled units into system free queue ***/
        EnterX25CriticalRegion(Adapter);
		k=pre->Next;
		pre->Next=Adapter->SysFreeQue;
		Adapter->SysFreeQue=lc->RecvQue;

		/*** delete the assembled units from local receive queue ***/
		lc->RecvQue=k;
		if (lc->RecvQue==LAST_PKT)
		 lc->RecvTail=LAST_PKT;
        OutX25CriticalRegion(Adapter);

		break;
  }
  /*enable();*/
  return ((int)RecvLen);
}

void  DiscardPacket(
		IN PNE2000_ADAPTER    Adapter,
		struct LogicalChannel *lc)
{
  int k;
  struct PoolUnit *unit, *pre;

  if (lc->RecvQue==LAST_PKT)  {
	  return ;
  }

  unit=&Adapter->Pool[lc->RecvQue];
  do  {
	pre=unit;
	unit=&Adapter->Pool[unit->Next];
  }while (pre->Mbit && pre->Next!=LAST_PKT);

  /*** insert the assembled units into system free queue ***/
  k=pre->Next;
  pre->Next=Adapter->SysFreeQue;
  Adapter->SysFreeQue=lc->RecvQue;

  /*** delete the assembled units from local receive queue ***/
  lc->RecvQue=k;
  if (lc->RecvQue==LAST_PKT)
	 lc->RecvTail=LAST_PKT;
  return;
}



/********************************************************************
 NAME       : InitChannel

 DESCRIPTION:
		 The function is to initalize the channel information
 PARAMETERS :
		 (1)lcn: Logical channel number
 RETURN VALUE:
		 None
 *********************************************************************/

void InitChannel(u_char lcn)
{
  struct LogicalChannel *LC;

//  LC=&Channel[lcn];
  LC->LCGN=0;
  LC->LCN=lcn;
  LC->State=READY;
  LC->Cause=LC->DiagCode='\0';
  LC->Dbit='\0';
//  LC->Modul=Adapter->init.Modul;
  LC->RecvNo=LC->SendNo=0;
  if (LC->SendQue != LAST_PKT)
  {
//	Pool[LC->SendTail].Next=Adapter->SysFreeQue;
//	Adapter->SysFreeQue=LC->SendQue;
//	if (Adapter->init.PktLevState==POOL_OVER) Adapter->init.PktLevState=NORMAL;
  }
  LC->SendQue=LC->SendTail=LAST_PKT;
  if (LC->RecvQue != LAST_PKT)
  {
	//Pool[LC->RecvTail].Next=Adapter->SysFreeQue;
//	Adapter->SysFreeQue=LC->RecvQue;
//	if (Adapter->init.PktLevState==POOL_OVER) Adapter->init.PktLevState=NORMAL;
  }
  LC->RecvQue=LC->RecvTail=LAST_PKT;
  LC->SendWin.Low=LC->SendWin.High=0;
  LC->SendWin.Size=0;
  LC->Expedited=LC->TeleBusy=FALSE;
  LC->Fd=0;
  LC->tcp_len=0;

}





/*****************************************************************
 NAME       : GetPacket

 DESCRIPTION:
		 get a packet with requied type from the receive queue

 PARAMETERS :
       (1)LCN : Logical channel number
       (2)type: Packet type
       (3)pkt : packet pointer
		<< RETURNED PARAMETERS >>
 RETURN VALUE:
       (1) <0 : didn't find a packet with the required type
       (2) >=0: success
******************************************************************/
int  GetPacket(u_char LCN, char type, struct Packet *pkt)
{
  u_char *pstr1;
  u_int len, k;
  int i;
  struct LogicalChannel *LC;
  struct FdStruct *pfd;
  struct PoolUnit *unit;

  //LC=&Channel[LCN];
  pfd=&FdArray[LC->Fd];
//  unit=&Pool[LC->RecvQue];
  if (unit->Type != type)
  {
	 /*i=LC->RecvQue;
	 while(i!=LAST_PKT)
	 {
		char buf[20];
		unit=&Pool[i];
		sprintf(buf,"%d\n",unit->Type);
		d_prn(buf);
		i=unit->Next;
	 }*/
	 return NONE_PKT;
  }

  pstr1=unit->Data;
  pkt->Type=type;
  pkt->Head.Qbit=0;
  pkt->Head.Dbit=LC->Dbit;
  pkt->Head.Modul=LC->Modul;
  pkt->Head.LCGN=LC->LCGN;
  pkt->Head.LCN=LCN;

  switch (type)   {
	 case INCOM_CALL:
	 case CALL_CONN:
		/*** process incoming_call packet ***/
		/*** copy calling and called addresses ***/
		AddrCopy(&pkt->Body.Call.CallAddr, &pfd->CallAddr);
		AddrCopy(&pkt->Body.Call.CalledAddr, &pfd->CalledAddr);

		/*** copy facilities ***/
		k=(u_int)*pstr1++;
		pkt->Body.Call.Opt.Len=k;
		len=k+1;
		StrCopy(pkt->Body.Call.Opt.Buf, pstr1, k);
		pstr1+=k;

		/*** copy user_data ***/
		StrCopy(pkt->Body.Call.Data.Buf, pstr1, unit->Len-len);
		pkt->Body.Call.Data.Len=unit->Len-len;
		break;

	 case CLEAR_IND:
		/*** process clear_indication packet ***/
		/*** get clearing_cause & diagnostic_code ***/
		pkt->Body.Clear.Cause=LC->Cause;
		pkt->Body.Clear.DiagCode=LC->DiagCode;

		/*** get calling & called addresses ***
		AddrCopy(&pkt->Body.Clear.CallAddr, &pfd->CallAddr);
		AddrCopy(&pkt->Body.Clear.CalledAddr, &pfd->CalledAddr);

		** get facilities ***
		k=(u_int)*pstr1++;
		len=k+1;
		pkt->Body.Clear.Opt.Len=k;
		StrCopy(pkt->Body.Clear.Opt.Buf, pstr1, k);
		pstr1+=k;

		**** get clear_user_data ***
		pkt->Body.Clear.Data.Len=unit->Len-len;
		StrCopy(pkt->Body.Clear.Data.Buf, pstr1, unit->Len-len);*/
		break;

	 default:
		break;
  }

  /*** queue operate ***/
  disable();
  /*** insert the unit into system free queue ***/
  i=unit->Next;
//  unit->Next=Adapter->SysFreeQue;
 // Adapter->SysFreeQue=LC->RecvQue;

  /*** delete the unit from local receive queue ***/
  if (LC->RecvQue==LC->RecvTail)
		LC->RecvQue=LC->RecvTail=LAST_PKT;
  else
		LC->RecvQue=i;
  enable();

  return(GetPktSucc);
}







/*********************************************************
 NAME       : addrcpystb

 DESCRIPTION:
       The function is to change address string of ascii
		 into string of binary.
 PARAMETERS :
		 (1) souaddr : the source address.
		 (2) desaddr : the destination address.
 RETURN VALUE:
		 None
 *************************************************************/
void addrcpystb(struct NetAddr  *desaddr,struct NetAddr  *souaddr)
{
  int addrl;
  u_char  *ptrd, *ptrs;

  if (desaddr==NULL)
	 return;
  if (souaddr==NULL)  {
	 desaddr->Len=0;
	 desaddr->Addr[0]='\0';
	 return;
  }
  ptrd=desaddr->Addr;
  ptrs=souaddr->Addr;
  desaddr->Len=souaddr->Len;
  addrl=(int)souaddr->Len;
  while (addrl>0)    {
	 if (addrl>1)
		 *ptrd=((*ptrs-'0') << 4) | (*(ptrs+1)-'0');
	 else
		 *ptrd=(*ptrs-'0') << 4 ;
	 addrl=addrl-2;
	 ptrd++;
	 ptrs=ptrs+2;
  }
}




/*********************************************************
 NAME       : addrcpybts

 DESCRIPTION:
		 The function is to change address string of bsd into string of ascii.
 PARAMETERS :
		 (1) souaddr : the source address.
		 (2) desaddr : the destination address.
 RETURN VALUE:
		 None
 *************************************************************/
void addrcpybts(struct NetAddr  *desaddr,struct NetAddr  *souaddr)
{
  char addrl;
  u_char  *ptrd, *ptrs;
  if (desaddr==NULL)
	 return;
  if (souaddr==NULL)  {
	 desaddr->Len=0;
	 desaddr->Addr[0]='\0';
	 return;
  }
  ptrd=desaddr->Addr;
  ptrs=souaddr->Addr;
  desaddr->Len=souaddr->Len;
  addrl=souaddr->Len;
  while (addrl>0)  {
	 *ptrd=(*ptrs >> 4) + '0';
	 *ptrd++;
	 *ptrd=(*ptrs & 0x0f ) + '0';
	 *ptrd++;
	 addrl=addrl-2;
	 ptrs=ptrs+1;
  }
}




/*********************************************************
 NAME       : sendrestart

 DESCRIPTION:
       The function is to send restart packet.
 PARAMETERS :
       see function comment.
 RETURN VALUE:
       return the value message of error.
 *************************************************************/

int sendrestart(IN PNE2000_ADAPTER    Adapter,
				IN int fd)
{
  u_char lcn;                        /* circal variable */
  int ret;
  struct Packet pac;
  struct PoolUnit unit;


  if (Adapter->init.PktLevState!=NORMAL)        /* Packet level state  error. */
    return(SndRstFail);

  lcn=FdArray[fd].LCN;
  pac.Type=RST_REQ;
  pac.Head.Qbit=1;
  pac.Head.Dbit=0;
  ASSERT((0<=lcn)&&(lcn<VC_NUM));
  pac.Head.Modul=Adapter->Channel[lcn].Modul;
  pac.Head.LCGN=0;
  pac.Head.LCN=0;
  pac.Body.Restart.Cause=0;
  pac.Body.Restart.DiagCode=0;

  BindPacket(&pac,&unit);             /* bind packet  */

  unit.Len=5;
  unit.LCN='\0';
  unit.Type=RESTART_REQ;
  ret=SendToLink(Adapter,&unit);     /* send packet  */
  if (ret < 0)
    return ret;

  T20=Adapter->init.T20;                        /* set T23 timer */
  Adapter->Channel[lcn].State=RST_REQ;          /* set State of CLN */

  while ((TimeOut==FALSE) && (T20!=0)) /* !!! */
  {
  }

  if (TimeOut==TRUE)
  {
    TimeOut=FALSE;
    T20=0;
    return(T20TimeOut);
  }

  return(RestartSucc);
}





/*********************************************************
 NAME       : initfdarray

 DESCRIPTION:
		 The function is to initalize the fdarrray array.
 PARAMETERS :
		 see function comment.
 RETURN VALUE:
		 return the value message of error.
 *************************************************************/
void initfdarray(u_char i)
{
  FdArray[i].LCN=FDUNUSED;
  AddrCopy(&(FdArray[i].CallAddr),NULL);
  AddrCopy(&(FdArray[i].CalledAddr),NULL);
  FdArray[i].tcp_call=-1;
}
/*********************************************************
 NAME       : RestartAll
 *************************************************************/
void RestartAll(void)
{
 u_char ch;
 int i;

#ifdef _DEBUG
	d_prn("into restartall");
#endif
// Adapter->init.PktLevState=NORMAL;

 T20=T21=T22=T23=T28=0;

 /*** fd array & channel initalization.   ***/
 for(ch=0;ch<VC_NUM;ch++)    {
	 /*initfdarray(ch);*/
	 InitChannel(ch);
 }

 /*** system queues & buffer pool Adapter->init ***/
// Adapter->SysFreeQue=0;
// for (i=0; i<POOLLEN; i++)
//	 Pool[i].Next=i+1;
// Pool[POOLLEN-1].Next=LAST_PKT;

//The timer service is implemented by system callback procedure 
//The following initialization is omited
// TimeOut=FALSE;
// t_flag=0;
// r_flag=0;
#ifdef _DEBUG
 d_prn("out restartall");
#endif
}


//  The following defination is temporaily omited  by Guo  

    
/*void arg_err (void)
{
#ifdef _DEBUG
  d_prn("USAGE: x25pkt x25pktdrv_int_number recv_int_number lapb_int_no iprt_int_no");
  d_prn("sample: x25pkt 0x75 0x63 0x62 0x60");
#else
  printf("USAGE: x25pkt x25pktdrv_int_number recv_int_number lapb_int_no iprt_int_no\n");
  printf("sample: x25pkt 0x75 0x63 0x62 0x60\n");
#endif
}

int para_init()
{
 int i,count,tempi;
 char * cptr;

 unsigned int  * iptr;
 unsigned char  * cfptr;
 int int_num;

 for(i=0x50;i<0x100;i++)
 {
  iptr=(unsigned int  *)MK_FP(0,i*4);
  cfptr=(unsigned char  *)MK_FP(*(iptr+1),*iptr);
  cfptr+=FLAG_OFF;
  if((cfptr[0]=='B')&&(cfptr[1]=='S')&&
	 (cfptr[2]=='D')&&(cfptr[3]=='X')&&
	 (cfptr[4]=='2')&&(cfptr[5]=='5')&&
	 (cfptr[6]=='D')&&(cfptr[7]=='R')&&
	 (cfptr[8]=='V'))
  {
	 int_num=i;
	 break;
  }
 }
 if(i!=0x100)
 {
	 printf("The bsd x.25 driver had already instralled.\n");
	 printf("You must use \"x25init /un\" to uninstall it before you install it again.\n");
	 return -1;
 }


 cptr=para_buf;
 i=0;
 count=0;
 while(1)
 {
  while((*cptr==' ')&&(i<para_c))
  {
	*cptr='\0';
	cptr++;
	i++;
  }
  if(i==para_c)break;
  para_v[count]=cptr;
  count++;
  if(count==8)break;
  while((*cptr!=' ')&&(i<para_c))cptr++,i++;
  if(i==para_c)break;
 }
 if(count<4)
 {
	arg_err();
	return -1;
 }
 if(sscanf(para_v[0],"0x%x",&tempi)==1)
	 PKT_INT=tempi;
 else
 {
  arg_err();
  return -1;
 }
 if(sscanf(para_v[1],"0x%x",&tempi)==1)
	 RCV_INT=tempi;
 else
 {
  arg_err();
  return -1;
 }
 if(sscanf(para_v[2],"0x%x",&tempi)==1)
	 HDLC_INT=tempi;
 else
 {
  arg_err();
  return -1;
 }
 if(sscanf(para_v[3],"0x%x",&tempi)==1)
	 TCP_INT=tempi;
 else
 {
  arg_err();
  return -1;
 }
 TCP_OK=FALSE;
 Adapter->init.PktLevState=RESTARTING;
 return 0;
}    */

int ReSend(IN PNE2000_ADAPTER    Adapter,
		   IN struct LogicalChannel * LC)
{
	int i,j;
	i=LC->SendQue;
	
	while(i!=LAST_PKT)
	{
	 ASSERT((0<=i)&&(i<POOLLEN));
     j=CardPrepareTransmit(Adapter,0,Adapter->Pool[i].Data, Adapter->Pool[i].Len);
	 i=Adapter->Pool[i].Next;
	}
	return 1;
}




VOID MyUnicodeToAnsi(MY_ANSISTRING *mystring,
                     NDIS_STRING *ustring
                     )
{
  int i=0,len=0;
  u_char *ptr;
  ptr=ustring->Buffer;
  
  while (i<ustring->Length)
  {
   if( (*ptr)=='\0' && (*(ptr+1))=='\0')
     break;
   mystring->Buffer[len]=*ptr;
   i+=2;ptr+=2;
   len++;
   }
 
  mystring->Length=len;
  mystring->Buffer[len]='\0';

 }

VOID GetAddrPair(ADDR_PAIR_STRING *mystring,
                 NDIS_STRING *ustring
                     )
{
  int i=0,len=0;
  u_char *ptr;
  ptr=ustring->Buffer;
  
  while (i<ustring->Length)
  {
   if( (*ptr)=='\0' && (*(ptr+1))=='\0')
     break;
   mystring->Buffer[len]=*ptr;
   i+=2;ptr+=2;
   len++;
   }
 
  mystring->Length=len;
  mystring->Buffer[len]='\0';

 }


  





