VOID MyUnicodeToAnsi(MY_ANSISTRING *mystring,
                     NDIS_STRING *ustring);
VOID GetAddrPair(ADDR_PAIR_STRING *mystring,
                 NDIS_STRING *ustring
                     );


int	i2ssetwinsave(IN PNE2000_ADAPTER       Adapter);

void i2sresetwin(IN PNE2000_ADAPTER       Adapter);

PVOID i2sf2w(IN PNE2000_ADAPTER       Adapter,PVOID offset);

int i2ssetwin(IN PNE2000_ADAPTER       Adapter, unsigned short segment, unsigned short offset);

int		i2sloadsoftware(IN PNE2000_ADAPTER       Adapter);

void i2sreset(IN PNE2000_ADAPTER       Adapter);

void Cardconf(NDIS_HANDLE ConfigHandle, pc2card_scb_t *pc2card_scbp,ULONG *len);

int	i2sloadfsm(IN PNE2000_ADAPTER 	Adapter,ULONG	cmd,PVOID	param);

void i2sintr(IN PNE2000_ADAPTER       Adapter);

int pc2card_reset(IN PNE2000_ADAPTER Adapter, PUCHAR v_dpram, unsigned short st);

int	licopyin(PUCHAR src,PUCHAR	dec,ULONG	len);

int	getbaud(UCHAR	baud,UCHAR sccclk,PUCHAR timelow,PUCHAR timehigh);

//return value	>0 Bytes remail not send don't worry it won't happen
//				=0 send complete
//<0 some error
#define	I2SERR_LENOVER	-1	//BytesToSend > max link send data
#define	I2SERR_BUFFULL	-2	//card buf full
#define	I2SERR_CARDERR	-3	//card error There must be something wrong with the software!
int
CardPrepareTransmit(
    IN PNE2000_ADAPTER       Adapter,
    IN USHORT               CardLine,    /* HTDSU_CMD_LINE1 or HTDSU_CMD_LINE2 */
	IN PUCHAR				buf_ptr,
    IN int               BytesToSend
    );

VOID
HtDsuReceivePacket(
    IN PNE2000_ADAPTER   Adapter,
    IN USHORT	CardLine
    );

VOID HtDsuHandlePacket(
	IN PNE2000_ADAPTER    Adapter,
	IN PUCHAR             Data_buf,
	IN UINT               Data_len
    );
    

NDIS_STATUS  CardStart(    IN PNE2000_ADAPTER       Adapter);

#ifndef min
#define min(x,y)	((x<y)?(x):(y))
#endif
