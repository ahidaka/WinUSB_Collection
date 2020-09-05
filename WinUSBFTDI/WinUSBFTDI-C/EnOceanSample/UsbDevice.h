#pragma once
//
// WinUSBDriver.h
//
#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <winusb.h>
#include <usb.h>
#include <initguid.h>

//
// Device Interface GUID.
//
// {BB545E12-7723-4ED0-BD99-1F64B6E45BE6}
DEFINE_GUID(GUID_DEVINTERFACE_WinUSBApplication,
    0xbb545e12, 0x7723, 0x4ed0, 0xbd, 0x99, 0x1f, 0x64, 0xb6, 0xe4, 0x5b, 0xe6);

typedef struct _DEVICE_DATA {

    BOOL                    HandlesOpen;
    WINUSB_INTERFACE_HANDLE WinusbHandle;
    HANDLE                  DeviceHandle;
    TCHAR                   DevicePath[MAX_PATH];
    UCHAR                   ReadPipe;
    UCHAR                   WritePipe;
    BOOL                    BlockRead;

}
DEVICE_DATA, *PDEVICE_DATA;

HRESULT
UsbDeviceOpen(
    _Out_       PDEVICE_DATA DeviceData,
    _Out_opt_   PBOOL        FailureDeviceNotFound
    );

VOID
UsbDeviceClose(
    _Inout_     PDEVICE_DATA DeviceData
    );

//BOOL
//UsbDeviceInspect(
//    _In_        PDEVICE_DATA DeviceData,
//    _In_        ULONG BaudRate
//);

BOOL
UsbDeviceInitialize(
    _In_        PDEVICE_DATA DeviceData,
    _In_        ULONG BaudRate
);

BOOL
UsbDeviceControlTransfer(
    _In_        PDEVICE_DATA DeviceData,
    _In_        UCHAR   RequestType,
    _In_        UCHAR   Request,
    _In_        USHORT  Value,
    _In_        USHORT  Index
);

BOOL
UsbDeviceWrite(
    _In_        PDEVICE_DATA DeviceData,
    _In_        UCHAR* Buffer,
    _In_        ULONG Length,
    _Out_opt_   ULONG* pcbWritten
);

BOOL
UsbDeviceRead(
    _In_        PDEVICE_DATA DeviceData,
    _Out_       UCHAR* Buffer,
    _In_        ULONG Length,
    _Out_opt_   ULONG* pcbRead
);
