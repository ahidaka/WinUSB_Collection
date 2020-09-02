#pragma once

#include "pch.h"

//#define RX_BUFSIZE (0x40 * 4)
#define RX_BUFSIZE (0x40)

typedef enum rx_staus
{
    RX_EMPTY = 0,
    RX_RECEIVED = 1,
    RX_OVER_FLOW = 2,
}
RX_STATUS;

typedef struct _usb_buffer
{
    RX_STATUS RxStatus;
    UINT RxSystem;
    UINT RxUser;
    UINT RxCount;
    UCHAR RxBuffer[RX_BUFSIZE];
} USB_BUFFER, * PUSB_BUFFER;


BOOL BufferInitialize(VOID);

BOOL BufferOutputTo(UCHAR Data);

BOOL BufferInputFrom(PUCHAR pData);

VOID BufferPrint(VOID);
