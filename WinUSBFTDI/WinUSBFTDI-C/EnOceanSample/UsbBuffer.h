#pragma once
//
// WinUSBBuffer.h
//
#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <winusb.h>
#include <usb.h>
#include <initguid.h>

//#define USB_RX_BUFSIZE (0x40 * 4)
#define USB_RX_BUFSIZE (0x40)

typedef enum rx_staus
{
    USB_RX_EMPTY = 0,
    USB_RX_RECEIVED = 1,
    USB_RX_OVERFLOW = 2,
}
USB_RX_STATUS;

typedef struct _usb_buffer
{
    USB_RX_STATUS RxStatus;
    UINT RxSystem;
    UINT RxUser;
    UINT RxCount;
    UCHAR RxBuffer[USB_RX_BUFSIZE];
} USB_BUFFER, * PUSB_BUFFER;


BOOL UsbBufferInitialize(VOID);

BOOL UsbBufferOutputTo(UCHAR Data);

BOOL UsbBufferInputFrom(PUCHAR pData);

VOID UsbBufferPrint(VOID);
