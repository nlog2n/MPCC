/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    keywords.h

Abstract:

    Contains all Ndis2 and Ndis3 mac-specific keywords.

Author:

    Bob Noradki

Environment:

    This driver is expected to work in DOS, OS2 and NT at the equivalent
    of kernal mode.

    Architecturally, there is an assumption in this driver that we are
    on a little endian machine.

Notes:

    optional-notes

Revision History:



--*/
#ifndef HTDSU_MEMORY_SIZE
#define	HTDSU_MEMORY_SIZE	0x4000
#endif

#define HTDSU_PARAM_IRQ03               03
#define HTDSU_PARAM_IRQ04               04
#define HTDSU_PARAM_IRQ05               05
#define HTDSU_PARAM_IRQ09               9
#define HTDSU_PARAM_IRQ10               10
#define HTDSU_PARAM_IRQ11               11
#define HTDSU_PARAM_IRQ12               12
#define HTDSU_PARAM_IRQ15               15

#define HTDSU_PARAM_RAMBASE1            (0x000D0000)
#define HTDSU_PARAM_RAMBASE2            (0x000D4000)
#define HTDSU_PARAM_RAMBASE3            (0x000D8000)
#define HTDSU_PARAM_RAMBASE4            (0x000C8000)



#ifndef NDIS2
#define NDIS2 0
#endif

#if NDIS2

#define IOADDRESS  NDIS_STRING_CONST("IOBASE")
#define INTERRUPT  NDIS_STRING_CONST("INTERRUPT")
#define MAX_MULTICAST_LIST  NDIS_STRING_CONST("MAXMULTICAST")
#define NETWORK_ADDRESS  NDIS_STRING_CONST("NETADDRESS")
#define BUS_TYPE  NDIS_STRING_CONST("BusType")

#else // NDIS3

#define IOADDRESS  NDIS_STRING_CONST("IoBaseAddress")
#define INTERRUPT  NDIS_STRING_CONST("InterruptNumber")
#define RAMBASE     NDIS_STRING_CONST("RamBaseAddress")
#define MAX_MULTICAST_LIST  NDIS_STRING_CONST("MaximumMulticastList")
#define NETWORK_ADDRESS  NDIS_STRING_CONST("NetworkAddress")
#define BUS_TYPE  NDIS_STRING_CONST("BusType")

#endif
