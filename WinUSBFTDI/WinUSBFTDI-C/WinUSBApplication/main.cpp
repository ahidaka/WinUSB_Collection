#include "pch.h"
#include <stdio.h>
#include <process.h> //system()

#if 0
typedef struct _WINUSB_SETUP_PACKET {
    UCHAR   RequestType;
    UCHAR   Request;
    USHORT  Value;
    USHORT  Index;
    USHORT  Length;
} WINUSB_SETUP_PACKET, * PWINUSB_SETUP_PACKET;
#endif

extern VOID UsbPrintData(PCHAR Name, PUCHAR Buffer, INT Size);

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
    BOOL                  result;
    BOOL                  noDevice;
    ULONG cbSize = 0;
    INT i;

    UNREFERENCED_PARAMETER(Argc);
    UNREFERENCED_PARAMETER(Argv);

    //
    // Find a device connected to the system that has WinUSB installed using our
    // INF
    //
    hr = UsbDeviceOpen(&deviceData, &noDevice);
    if (FAILED(hr)) {

        if (noDevice) {

            wprintf(L"Device not connected or driver not installed\n");

        } else {

            wprintf(L"Failed looking for device, HRESULT 0x%x\n", hr);
        }

        return 0;
    }

    result = UsbDeviceInitialize(&deviceData, 57600);
    if (!result)
    {
        printf("Error !result\n");
        goto done;
    }
#define TIMES 10

    for (i = 1; i < TIMES; i++)
    {
        UCHAR buffer[256];

        cbSize = 0;

        do
        {
            result = UsbDeviceRead(&deviceData, buffer, i /*length*/, &cbSize);
            if (!result)
            {
                printf("ERROR: !result\n");
                goto done;
            }
            printf("%2d: Got=%2d\n", i, cbSize);
            UsbPrintData("USR", buffer, (INT)cbSize);
        }
        while (cbSize == 0);
    }

    system("PAUSE");

done:
    UsbDeviceClose(&deviceData);
    return 0;
}
