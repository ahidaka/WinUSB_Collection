#include "pch.h"

#include <stdio.h>

#include <process.h> //system()

#include "buffer.h"


VOID PrintData(PCHAR Name, PUCHAR Buffer, INT Size)
{
    INT i;
    printf("%s:", Name);
    for (i = 0; i < Size; i++)
    {
        printf("%02X ", Buffer[i]);
    }
    printf("\n");
}

BOOL GetUSBDeviceSpeed(WINUSB_INTERFACE_HANDLE hDeviceHandle, UCHAR* pDeviceSpeed)
{
    if (!pDeviceSpeed || hDeviceHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    BOOL bResult = TRUE;

    ULONG length = sizeof(UCHAR);

    bResult = WinUsb_QueryDeviceInformation(hDeviceHandle, DEVICE_SPEED, &length, pDeviceSpeed);
    if (!bResult)
    {
        printf("Error getting device speed: %d.\n", GetLastError());
        goto done;
    }

    if (*pDeviceSpeed == LowSpeed)
    {
        printf("Device speed: %d (Low speed).\n", *pDeviceSpeed);
        goto done;
    }
    if (*pDeviceSpeed == FullSpeed)
    {
        printf("Device speed: %d (Full speed).\n", *pDeviceSpeed);
        goto done;
    }
    if (*pDeviceSpeed == HighSpeed)
    {
        printf("Device speed: %d (High speed).\n", *pDeviceSpeed);
        goto done;
    }

done:
    return bResult;
}

struct PIPE_ID
{
    UCHAR  PipeInId;
    UCHAR  PipeOutId;
};

BOOL QueryDeviceEndpoints(WINUSB_INTERFACE_HANDLE hDeviceHandle, PIPE_ID* pipeid)
{
    if (hDeviceHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    BOOL bResult = TRUE;

    USB_INTERFACE_DESCRIPTOR InterfaceDescriptor;
    ZeroMemory(&InterfaceDescriptor, sizeof(USB_INTERFACE_DESCRIPTOR));

    WINUSB_PIPE_INFORMATION  Pipe;
    ZeroMemory(&Pipe, sizeof(WINUSB_PIPE_INFORMATION));


    bResult = WinUsb_QueryInterfaceSettings(hDeviceHandle, 0, &InterfaceDescriptor);

    if (bResult)
    {
        for (int index = 0; index < InterfaceDescriptor.bNumEndpoints; index++)
        {
            bResult = WinUsb_QueryPipe(hDeviceHandle, (UCHAR)0, (UCHAR)index, &Pipe);

            if (bResult)
            {
                if (Pipe.PipeType == UsbdPipeTypeControl)
                {
                    printf("Endpoint index: %d, Pipe type: Control, Pipe ID: %X.\n", index, Pipe.PipeId);
                }
                if (Pipe.PipeType == UsbdPipeTypeIsochronous)
                {
                    printf("Endpoint index: %d, Pipe type: Isochronous, Pipe ID: %X.\n", index, Pipe.PipeId);
                }
                if (Pipe.PipeType == UsbdPipeTypeBulk)
                {
                    if (USB_ENDPOINT_DIRECTION_IN(Pipe.PipeId))
                    {
                        printf("Endpoint index: %d, Pipe type: Bulk IN, Pipe ID: %X.\n", index, Pipe.PipeId);
                        pipeid->PipeInId = Pipe.PipeId;
                    }
                    if (USB_ENDPOINT_DIRECTION_OUT(Pipe.PipeId))
                    {
                        printf("Endpoint index: %d, Pipe type: Bulk OUT, Pipe ID: %X.\n", index, Pipe.PipeId);
                        pipeid->PipeOutId = Pipe.PipeId;
                    }

                }
                if (Pipe.PipeType == UsbdPipeTypeInterrupt)
                {
                    printf("Endpoint index: %d, Pipe type: Interrupt, Pipe ID: %X.\n", index, Pipe.PipeId);
                }
            }
            else
            {
                continue;
            }
        }
    }

    return bResult;
}

BOOL SendDatatoDefaultEndpoint(WINUSB_INTERFACE_HANDLE hDeviceHandle)
{
    if (hDeviceHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    BOOL bResult = TRUE;


    UCHAR bars = 0;

    WINUSB_SETUP_PACKET SetupPacket;
    ZeroMemory(&SetupPacket, sizeof(WINUSB_SETUP_PACKET));
    ULONG cbSent = 0;

    //Set bits to light alternate bars
    for (short i = 0; i < 7; i += 2)
    {
        bars += 1 << i;
    }

    //Create the setup packet
    SetupPacket.RequestType = 0;
    SetupPacket.Request = 0xD8;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = sizeof(UCHAR);

    bResult = WinUsb_ControlTransfer(hDeviceHandle, SetupPacket, &bars, sizeof(UCHAR), &cbSent, 0);
    if (!bResult)
    {
        goto done;
    }

    printf("Data sent: %zd \nActual data transferred: %d.\n", sizeof(bars), cbSent);


done:
    return bResult;

}

#if 0
typedef struct _WINUSB_SETUP_PACKET {
    UCHAR   RequestType;
    UCHAR   Request;
    USHORT  Value;
    USHORT  Index;
    USHORT  Length;
} WINUSB_SETUP_PACKET, * PWINUSB_SETUP_PACKET;
#endif


BOOL ControlWrite(
    WINUSB_INTERFACE_HANDLE hDeviceHandle,
    UCHAR   RequestType,
    UCHAR   Request,
    USHORT  Value,
    USHORT  Index)
{
    BOOL bResult = TRUE;
    ULONG cbSent = 0;
    WINUSB_SETUP_PACKET SetupPacket;

    if (hDeviceHandle == INVALID_HANDLE_VALUE)
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

    bResult = WinUsb_ControlTransfer(hDeviceHandle, SetupPacket, NULL, 0, &cbSent, 0);
    if (!bResult)
    {
        goto done;
    }

    printf("Data sent: Actual data transferred: %d.\n", cbSent);


done:
    return bResult;

}


INT GetDivisorValue(INT BaudRate)
{
    INT frequency = 48 * 1000 * 1000;
    INT divisor;
    INT moduler;
    INT rounder;

    divisor = frequency / BaudRate / 16;
    moduler = (frequency / BaudRate / 2) % 8;
    rounder = (7 - moduler) / 2;

    //printf("divisor=%d 0x%06X\n", divisor, divisor);
    //printf("moduler=%d 0x%06X\n", moduler, moduler);
    //printf("rounder=%d 0x%06X\n", rounder, rounder);

    return(divisor | (rounder << 14));
}

BOOL InitializeSettings(WINUSB_INTERFACE_HANDLE hDeviceHandle, INT BaudRate)
{
    BOOL bResult = TRUE;
    INT divisor;

    if (hDeviceHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    divisor = GetDivisorValue(BaudRate);

    (VOID) BufferInitialize();

    ControlWrite(hDeviceHandle, 0x40, 0, 0, 0);
    ControlWrite(hDeviceHandle, 0x40, 0, 1, 0);
    ControlWrite(hDeviceHandle, 0x40, 0, 2, 0);
    ControlWrite(hDeviceHandle, 0x40, 2, 0x0000, 0);
    ControlWrite(hDeviceHandle, 0x40, 3, (USHORT) divisor, 0);
    ControlWrite(hDeviceHandle, 0x40, 4, 0x0008, 0); // data bit 8, parity none, stop bit 1, tx off

    return bResult;

}

BOOL WriteToBulkEndpoint(
    WINUSB_INTERFACE_HANDLE hDeviceHandle,
    UCHAR* pID,
    UCHAR* Buffer,
    ULONG Length,
    ULONG* pcbWritten)
{
    BOOL bResult = TRUE;
    ULONG cbSent = 0;
    const size_t cbSize = 0x40;
    UCHAR szBuffer[cbSize];

    if (hDeviceHandle == INVALID_HANDLE_VALUE || !pID || !pcbWritten)
    {
        return FALSE;
    }

    while (cbSent < Length)
    {
        bResult = WinUsb_WritePipe(hDeviceHandle, *pID, &Buffer[cbSent], cbSize, &cbSent, 0);
        if (!bResult)
        {
            goto done;
        }
        Length -= cbSent;
    }

    printf("Wrote to pipe %d: %s \nActual data transferred: %d.\n", *pID, szBuffer, cbSent);
    *pcbWritten = cbSent;

done:
    return bResult;

}


BOOL ReadFromBulkEndpoint(
    WINUSB_INTERFACE_HANDLE hDeviceHandle,
    UCHAR* pID,
    UCHAR* Buffer,
    ULONG Length,
    ULONG* pcbRead)
{
    const INT packetSize = 0x40;
    const INT headerSize = 2;
    BOOL bResult = TRUE;
    ULONG i;
    ULONG dataLength;
    ULONG cbRead;
    UCHAR szBuffer[packetSize];

    if (hDeviceHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    BufferPrint();

     // Read old buffer data, if exists
    for (dataLength = 0; dataLength < Length; dataLength++)
    {
        if (!BufferInputFrom(&Buffer[dataLength]))
        {
            break;
        }
        printf("1:Buffer[%d]=%02X\n", dataLength, Buffer[dataLength]);
    }

    while (dataLength < Length)
    {
        //
        // Read One packet from pipe
        //
        cbRead = 0;
        bResult = WinUsb_ReadPipe(hDeviceHandle, *pID, szBuffer, packetSize, &cbRead, 0);
        if (!bResult)
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
            PrintData("USB", szBuffer, (INT)cbRead);
            BufferPrint();

            cbRead -= headerSize;
            for (i = 0; i < cbRead; i++)
            {
                if (!BufferOutputTo(szBuffer[i]))
                {
                    // maybe overflow
                    printf("maybe overflow, i=%d\n", i);
                    break;
                }
            }

            // Read old buffer data, if exists
            for (i = 0; dataLength < Length && i < cbRead; dataLength++, i++)
            {
                if (!BufferInputFrom(&Buffer[dataLength]))
                {
                    printf("InputFromBuffer error, i=%d\n", i);
                    break;
                }

                printf("2:Buffer[%d]=%02X\n", dataLength, Buffer[dataLength]);

            }
        }
        //else
        //{
        //    printf("InputFromBuffer error, i=%d\n", cbRead);
        //    break;
        //}
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

    BufferPrint();

    return bResult;

}


LONG __cdecl
_tmain(
    LONG     Argc,
    LPTSTR * Argv
    )
/*++

Routine description:

    Sample program that communicates with a USB device using WinUSB

--*/
{
    DEVICE_DATA           deviceData;
    HRESULT               hr;
    USB_DEVICE_DESCRIPTOR deviceDesc;
    BOOL                  bResult;
    BOOL                  noDevice;
    ULONG                 lengthReceived;

    PIPE_ID PipeID;
    UCHAR DeviceSpeed;
    ULONG cbSize = 0;

    INT i;

    UNREFERENCED_PARAMETER(Argc);
    UNREFERENCED_PARAMETER(Argv);

    //
    // Find a device connected to the system that has WinUSB installed using our
    // INF
    //
    hr = OpenDevice(&deviceData, &noDevice);

    if (FAILED(hr)) {

        if (noDevice) {

            wprintf(L"Device not connected or driver not installed\n");

        } else {

            wprintf(L"Failed looking for device, HRESULT 0x%x\n", hr);
        }

        return 0;
    }

    //
    // Get device descriptor
    //
    bResult = WinUsb_GetDescriptor(deviceData.WinusbHandle,
                                   USB_DEVICE_DESCRIPTOR_TYPE,
                                   0,
                                   0,
                                   (PBYTE) &deviceDesc,
                                   sizeof(deviceDesc),
                                   &lengthReceived);

    if (FALSE == bResult || lengthReceived != sizeof(deviceDesc)) {

        wprintf(L"Error among LastError %d or lengthReceived %d\n",
                FALSE == bResult ? GetLastError() : 0,
                lengthReceived);
        CloseDevice(&deviceData);
        return 0;
    }

    bResult = GetUSBDeviceSpeed(deviceData.WinusbHandle, &DeviceSpeed);
    if (!bResult)
    {
        goto done;
    }

    bResult = QueryDeviceEndpoints(deviceData.WinusbHandle, &PipeID);
    if (!bResult)
    {
        goto done;
    }

    //
    // Print a few parts of the device descriptor
    //
    wprintf(L"Device found: VID_%04X&PID_%04X; bcdUsb %04X\n",
        deviceDesc.idVendor,
        deviceDesc.idProduct,
        deviceDesc.bcdUSB);



    //
    //
    //
    bResult = InitializeSettings(deviceData.WinusbHandle, 57600);
    if (!bResult)
    {
        goto done;
    }

#if 0 // Control
    bResult = SendDatatoDefaultEndpoint(deviceData.WinusbHandle);
    if (!bResult)
    {
        goto done;
    }
#endif

#if 0
    bResult = WriteToBulkEndpoint(deviceData.WinusbHandle, &PipeID.PipeOutId, &cbSize);
    if (!bResult)
    {
        goto done;
    }
#endif

#if 0
    do
    {
#if 0
        BOOL ReadFromBulkEndpoint(
            WINUSB_INTERFACE_HANDLE hDeviceHandle,
            UCHAR * pID,
            UCHAR * Buffer,
            ULONG Length,
            ULONG * pcbRead)
#endif
        BOOL gotHeader = FALSE;
        UCHAR buffer[256];
        INT length = 128;

        bResult = ReadFromBulkEndpoint(deviceData.WinusbHandle, &PipeID.PipeInId, buffer, length, &cbSize);
        if (!bResult)
        {
            printf("ERROR: !bResult\n");
            goto done;
        }

        PrintData("USR", buffer, (INT)cbSize);

        gotHeader = cbSize == 0x55;

        Sleep(100);

    }
    while (!gotHeader);

#endif
#define TIMES 10

    for (i = 1; i < TIMES; i++)
    {
        UCHAR buffer[256];

        cbSize = 0;

        do
        {
            bResult = ReadFromBulkEndpoint(deviceData.WinusbHandle, &PipeID.PipeInId, buffer, i /*length*/, &cbSize);
            if (!bResult)
            {
                printf("ERROR: !bResult\n");
                goto done;
            }
            printf("%2d: Got=%2d\n", i, cbSize);
            PrintData("USR", buffer, (INT)cbSize);
        }
        while (cbSize == 0);
    }

    system("PAUSE");

done:
    CloseDevice(&deviceData);
    return 0;
}
