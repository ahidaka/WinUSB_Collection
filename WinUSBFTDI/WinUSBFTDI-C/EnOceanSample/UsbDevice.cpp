#include "UsbDevice.h"
#include "UsbBuffer.h"
#include <cfgmgr32.h>

//
//
//
VOID UsbPrintData(PCHAR Name, PUCHAR Buffer, INT Size);

//
//
//
HRESULT
RetrieveDevicePath(
    _Out_bytecap_(BufLen) LPTSTR DevicePath,
    _In_                  ULONG  BufferLength,
    _Out_opt_             PBOOL  DeviceNotfound
)
{
    CONFIGRET cr = CR_SUCCESS;
    HRESULT   hr = S_OK;
    PTSTR     DeviceInterfaceList = NULL;
    ULONG     DeviceInterfaceListLength = 0;

    do
    {
        cr = CM_Get_Device_Interface_List_Size(&DeviceInterfaceListLength,
            (LPGUID)&GUID_DEVINTERFACE_WinUSBApplication,
            NULL,
            CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

        if (cr != CR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(CM_MapCrToWin32Err(cr, ERROR_INVALID_DATA));
            break;
        }

        DeviceInterfaceList = (PTSTR)HeapAlloc(GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            DeviceInterfaceListLength * sizeof(TCHAR));

        if (DeviceInterfaceList == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        cr = CM_Get_Device_Interface_List((LPGUID)&GUID_DEVINTERFACE_WinUSBApplication,
            NULL,
            DeviceInterfaceList,
            DeviceInterfaceListLength,
            CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

        if (cr != CR_SUCCESS)
        {
            HeapFree(GetProcessHeap(), 0, DeviceInterfaceList);

            if (cr != CR_BUFFER_SMALL)
            {
                hr = HRESULT_FROM_WIN32(CM_MapCrToWin32Err(cr, ERROR_INVALID_DATA));
            }
        }
    }
    while (cr == CR_BUFFER_SMALL);

    if (FAILED(hr))
    {
        return hr;
    }

    if (*DeviceInterfaceList == TEXT('\0'))
    {
        if (DeviceNotfound != NULL)
        {
            *DeviceNotfound = TRUE;
        }

        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        HeapFree(GetProcessHeap(), 0, DeviceInterfaceList);
        return hr;
    }

    hr = StringCbCopy(DevicePath,
        BufferLength,
        DeviceInterfaceList);

    HeapFree(GetProcessHeap(), 0, DeviceInterfaceList);

    if (DeviceNotfound != NULL)
    {
        *DeviceNotfound = FALSE;
    }
    return hr;
}

BOOL QueryDeviceEndpoints(
    _In_        PDEVICE_DATA DeviceData
)
{
    INT index;
    BOOL result = TRUE;
    USB_INTERFACE_DESCRIPTOR InterfaceDescriptor;
    WINUSB_PIPE_INFORMATION  Pipe;

    if (DeviceData->WinusbHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    ZeroMemory(&InterfaceDescriptor, sizeof(USB_INTERFACE_DESCRIPTOR));
    ZeroMemory(&Pipe, sizeof(WINUSB_PIPE_INFORMATION));

    result = WinUsb_QueryInterfaceSettings(DeviceData->WinusbHandle, 0, &InterfaceDescriptor);
    if (result)
    {
        for (index = 0; index < InterfaceDescriptor.bNumEndpoints; index++)
        {
            result = WinUsb_QueryPipe(DeviceData->WinusbHandle, (UCHAR)0, (UCHAR)index, &Pipe);
            if (result)
            {
                switch (Pipe.PipeType)
                {
                case UsbdPipeTypeControl:
                    printf("Endpoint index: %d, Pipe type: Control, Pipe ID: %X.\n", index, Pipe.PipeId);
                    break;
                case UsbdPipeTypeInterrupt:
                    printf("Endpoint index: %d, Pipe type: Interrupt, Pipe ID: %X.\n", index, Pipe.PipeId);
                    break;
                case UsbdPipeTypeBulk:
                    if (USB_ENDPOINT_DIRECTION_IN(Pipe.PipeId))
                    {
                        printf("Endpoint index: %d, Pipe type: Bulk IN, Pipe ID: %X.\n", index, Pipe.PipeId);
                        DeviceData->ReadPipe = Pipe.PipeId;
                    }
                    else if (USB_ENDPOINT_DIRECTION_OUT(Pipe.PipeId))
                    {
                        printf("Endpoint index: %d, Pipe type: Bulk OUT, Pipe ID: %X.\n", index, Pipe.PipeId);
                        DeviceData->WritePipe = Pipe.PipeId;
                    }
                    break;
                case UsbdPipeTypeIsochronous:
                    printf("Endpoint index: %d, Pipe type: Isochronous, Pipe ID: %X.\n", index, Pipe.PipeId);
                    break;
                }
            }
            else
            {
                continue;
            }
        }
    }

    return result;
}

//
//
//
BOOL DeviceInspect(
    _In_        PDEVICE_DATA DeviceData
)
{
    USB_DEVICE_DESCRIPTOR deviceDesc;
    ULONG   receivedLength;
    BOOL    result;

    result = WinUsb_GetDescriptor(DeviceData->WinusbHandle,
        USB_DEVICE_DESCRIPTOR_TYPE,
        0,
        0,
        (PBYTE)&deviceDesc,
        sizeof(deviceDesc),
        &receivedLength);

    if (!result || receivedLength != sizeof(deviceDesc)) {
        wprintf(L"Error among LastError %d or lengthReceived %d\n",
            !result ? GetLastError() : 0,
            receivedLength);
        UsbDeviceClose(DeviceData);
        return FALSE;
    }

    wprintf(L"Device found: VID_%04X&PID_%04X; bcdUsb %04X\n",
        deviceDesc.idVendor,
        deviceDesc.idProduct,
        deviceDesc.bcdUSB);

    result = QueryDeviceEndpoints(DeviceData);
    return result;
}

HRESULT
UsbDeviceOpen(
    _Out_     PDEVICE_DATA DeviceData,
    _Out_opt_ PBOOL        DeviceNotFound
)
{
    HRESULT hr = S_OK;
    BOOL    result;

    DeviceData->HandlesOpen = FALSE;

    hr = RetrieveDevicePath(DeviceData->DevicePath,
        sizeof(DeviceData->DevicePath),
        DeviceNotFound);
    if (FAILED(hr))
    {
        return hr;
    }

    DeviceData->DeviceHandle = CreateFile(DeviceData->DevicePath,
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        NULL);

    if (DeviceData->DeviceHandle == INVALID_HANDLE_VALUE)
    {

        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    result = WinUsb_Initialize(DeviceData->DeviceHandle,
        &DeviceData->WinusbHandle);
    if (!result)
    {

        hr = HRESULT_FROM_WIN32(GetLastError());
        CloseHandle(DeviceData->DeviceHandle);
        return hr;
    }

    DeviceData->BlockRead = FALSE;
    DeviceData->HandlesOpen = TRUE;
    return hr;
}

VOID
UsbDeviceClose(
    _Inout_ PDEVICE_DATA DeviceData
)
{
    if (FALSE == DeviceData->HandlesOpen) {
        return;
    }

    WinUsb_Free(DeviceData->WinusbHandle);
    CloseHandle(DeviceData->DeviceHandle);
    DeviceData->HandlesOpen = FALSE;

    return;
}

BOOL
UsbDeviceControlTransfer(
    _In_        PDEVICE_DATA DeviceData,
    _In_        UCHAR   RequestType,
    _In_        UCHAR   Request,
    _In_        USHORT  Value,
    _In_        USHORT  Index
)
{
    BOOL result = TRUE;
    ULONG cbSent = 0;
    WINUSB_SETUP_PACKET SetupPacket;

    if (DeviceData->WinusbHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    ZeroMemory(&SetupPacket, sizeof(WINUSB_SETUP_PACKET));

    //Create the setup packet
    SetupPacket.RequestType = RequestType;
    SetupPacket.Request = Request;
    SetupPacket.Value = Value;
    SetupPacket.Index = Index;
    SetupPacket.Length = 0;

    result = WinUsb_ControlTransfer(DeviceData->WinusbHandle,
        SetupPacket, NULL, 0, &cbSent, 0);
    if (result)
    {
        printf("Data sent: Actual data transferred: %d.\n", cbSent);
    }
    return result;
}

ULONG GetDivisorValue(
    _In_        ULONG BaudRate
)
{
    ULONG frequency = 48 * 1000 * 1000;
    ULONG divisor;
    ULONG moduler;
    ULONG rounder;

    divisor = frequency / BaudRate / 16;
    moduler = (frequency / BaudRate / 2) % 8;
    rounder = (7 - moduler) / 2;

    //printf("divisor=%d 0x%06X\n", divisor, divisor);
    //printf("moduler=%d 0x%06X\n", moduler, moduler);
    //printf("rounder=%d 0x%06X\n", rounder, rounder);

    return(divisor | (rounder << 14));
}

BOOL
UsbDeviceInitialize(
    _In_        PDEVICE_DATA DeviceData,
    _In_        ULONG BaudRate
)
{
    BOOL result;
    INT divisor;

    if (DeviceData->WinusbHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    result = DeviceInspect(DeviceData);
    if (!result)
    {
        return FALSE;
    }

    divisor = GetDivisorValue(BaudRate);

    (VOID)UsbBufferInitialize();

    UsbDeviceControlTransfer(DeviceData, 0x40, 0, 0, 0);
    UsbDeviceControlTransfer(DeviceData, 0x40, 0, 1, 0);
    UsbDeviceControlTransfer(DeviceData, 0x40, 0, 2, 0);
    UsbDeviceControlTransfer(DeviceData, 0x40, 2, 0x0000, 0);
    UsbDeviceControlTransfer(DeviceData, 0x40, 3, (USHORT)divisor, 0);
    // DataBit 8, Parity none, StopBit 1
    UsbDeviceControlTransfer(DeviceData, 0x40, 4, 0x0008, 0);
    
    return result;
}

BOOL
UsbDeviceWrite(
    _In_        PDEVICE_DATA DeviceData,
    _In_        UCHAR* Buffer,
    _In_        ULONG Length,
    _Out_opt_   ULONG* writtenLength
)
{
    BOOL result = TRUE;
    const size_t packetSize = 0x40;
    ULONG sentLength = 0;
    ULONG packetLength;
    ULONG sentPacket;

    if (DeviceData->DeviceHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    while (sentLength < Length)
    {
        packetLength = Length - sentLength;
        if (packetLength > packetSize)
        {
            packetLength = packetSize;
        }
        sentPacket = 0;
        result = WinUsb_WritePipe(
            DeviceData->WinusbHandle,
            DeviceData->WritePipe,
            &Buffer[sentLength],
            packetSize,
            &sentPacket,
            0
        );
        if (!result)
        {
            return result;
        }
        if (sentPacket > packetLength)
        {
            sentPacket = packetLength;
        }
        Length -= sentPacket;
        sentLength += sentPacket;
    }

    printf("UsbDeviceWrite: Actual data transferred: %d\n", sentLength);
    if (writtenLength != NULL)
    {
        *writtenLength = sentLength;
    }

    return result;
}


BOOL
UsbDeviceRead(
    _In_        PDEVICE_DATA DeviceData,
    _Out_       UCHAR* Buffer,
    _In_        ULONG Length,
    _Out_opt_   ULONG* pcbRead
)
{
    const INT packetSize = 0x40;
    const INT headerSize = 2;
    BOOL result = TRUE;
    ULONG i;
    ULONG dataLength;
    ULONG cbRead;
    UCHAR szBuffer[packetSize];

    if (DeviceData->WinusbHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    //printf("Read Length=%lu\n", Length); ////////
    UsbBufferPrint();

    // Read old buffer data, if exists
    for (dataLength = 0; dataLength < Length; dataLength++)
    {
        if (!UsbBufferInputFrom(&Buffer[dataLength]))
        {
            break;
        }
        printf("1:Buffer[%d]=%02X\n", dataLength, Buffer[dataLength]);
    }

    //printf("End for dataLength=%lu\n", dataLength); ////////
    while (dataLength < Length)
    {
        //
        // Read One packet from pipe
        //
        cbRead = 0;
        result = WinUsb_ReadPipe(
            DeviceData->WinusbHandle,
            DeviceData->ReadPipe,
            szBuffer,
            packetSize,
            &cbRead,
            0
        );
        if (!result)
        {
            printf("WinUsb_ReadPipe error\n");
            break;
        }

        if (cbRead != headerSize)
        {
            printf("WinUsb_ReadPipe: Length=%lu\n", cbRead);
        }
        //
        // Truncate header
        //
        if (cbRead > headerSize)
        {
            UsbPrintData("USB", szBuffer, (INT)cbRead);
            UsbBufferPrint();

            for (i = headerSize; i < cbRead; i++)
            {
                if (!UsbBufferOutputTo(szBuffer[i]))
                {
                    // maybe overflow
                    printf("maybe overflow, i=%d\n", i);
                    break;
                }
            }

            // Read old buffer data, if exists
            for (i = 0; dataLength < Length && i < cbRead; dataLength++, i++)
            {
                if (!UsbBufferInputFrom(&Buffer[dataLength]))
                {
                    printf("InputFromBuffer error, i=%d\n", i);
                    break;
                }

                printf("2:Buffer[%d]=%02X\n", dataLength, Buffer[dataLength]);

            }
        }
#if 1 // Non block read, 
        else
        {
        //    printf("InputFromBuffer error, i=%d\n", cbRead);
            break;
        }
#endif
    }

    if (!result)
    {
        printf("error\n");
    }

    //for (i = 0; i < dataLength; i++)
    //{
    //    printf("%02X ", szBuffer[i]);
    //}
    //printf("\n");


    if (pcbRead != NULL)
    {
        *pcbRead = dataLength;
    }

    UsbBufferPrint();

    //printf("Return Length=%lu\n", dataLength); ////////

    return result;
}


VOID UsbPrintData(PCHAR Name, PUCHAR Buffer, INT Size)
{
    INT i;
    printf("%s:", Name);
    for (i = 0; i < Size; i++)
    {
        printf("%02X ", Buffer[i]);
    }
    printf("\n");
}
