#include "pch.h"

#include <stdio.h>


UCHAR Crc8(PUCHAR Data, INT Count);

BOOL PreparationFilter(PDEVICE_DATA DeviceData)
{
    UNREFERENCED_PARAMETER(DeviceData);

    return TRUE;
}

BOOL MainLoop(PDEVICE_DATA DeviceData)
{
    const int BUFFER_SIZE = 64;
    UCHAR buffer[BUFFER_SIZE];
    ULONG length;
    BOOL result;
    BOOL gotHeader;
    USHORT dataLength;
    USHORT optionalLength;
    ULONG readLength;
    UCHAR packetType;
    UCHAR crc8h;
    UCHAR crc8d;

    printf("Enter MainLoop\n");
    do
    {
        dataLength = optionalLength = 0;
        gotHeader = FALSE;

        result = UsbDeviceRead(DeviceData, buffer, 1, &length);
        if (!result)
        {
            printf("ERROR: !result\n");
            return FALSE;
        }
        else if (length == 0)
        {
            Sleep(1);
            continue;
        }
        else if (buffer[0] != 0x55)
        {
            continue;
        }

        result = UsbDeviceRead(DeviceData, buffer, 5, &length);
        if (!result)
        {
            printf("ERROR: !result\n");
            return FALSE;
        }
        else if (length != 5)
        {
            Sleep(1);
            printf("TRY: length != 5\n"); ////
            continue;
        }

        dataLength = buffer[0] << 8 | buffer[1];
        optionalLength = buffer[2];
        packetType = buffer[3];
        crc8h = buffer[4];

        gotHeader = crc8h == Crc8(buffer, 4);
        printf("CRC8H: crc8h=%02X Calc=%02X\n", crc8h, Crc8(buffer, 4)); ////
    }
    while(!gotHeader);

    printf("Got Header!\n"); ////

    readLength = dataLength + optionalLength + 1;
    if (readLength > BUFFER_SIZE)
    {
        // mayne something error but have to keep buffer size
        readLength = BUFFER_SIZE;
    }

    UsbDeviceRead(DeviceData, buffer, readLength, &length);
    if (!result)
    {
        printf("ERROR: !result\n");
        return FALSE;
    }
    else if (length != readLength)
    {
        Sleep(1);
        return FALSE;
    }

    crc8d = buffer[readLength - 1];
    if (crc8d != Crc8(buffer, readLength - 1))
    {
        printf("CRC8D error!\n");
    }
    else
    {
        printf("CRC8D OK!\n");
    }
    printf("CRC8D: crc8d=%02X Calc=%02X\n", crc8d, Crc8(buffer, readLength - 1)); ////

    return TRUE;
}

static UCHAR crc8Table[] = 
    {
        0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15,
        0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
        0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
        0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
        0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5,
        0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
        0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85,
        0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
        0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
        0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
        0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2,
        0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
        0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32,
        0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
        0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
        0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
        0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c,
        0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
        0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec,
        0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
        0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
        0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
        0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c,
        0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
        0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b,
        0x76, 0x71, 0x78, 0x7f, 0x6A, 0x6d, 0x64, 0x63,
        0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
        0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
        0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb,
        0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8D, 0x84, 0x83,
        0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb,
        0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3
    };

UCHAR Crc8(PUCHAR Data, INT Offset, INT Count)
{
    UCHAR crc = 0;
    Count += Offset;
    for (int i = Offset; i < Count; i++)
        crc = crc8Table[crc ^ Data[i]];
    return crc;
}

UCHAR Crc8(PUCHAR Data, INT Count)
{
    return Crc8(Data, 0, Count);
}


LONG __cdecl
_tmain(
    LONG     Argc,
    LPTSTR * Argv
    )
{
    DEVICE_DATA           deviceData;
    HRESULT               hr;
    BOOL                  result;
    BOOL                  noDevice;

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
        UsbDeviceClose(&deviceData);
        return 0;
    }

    result = UsbDeviceInitialize(&deviceData, 57600);
    if (!result)
    {
        printf("Error !result\n");
        UsbDeviceClose(&deviceData);
        return 0;
    }

    if (!PreparationFilter(&deviceData))
    {
        printf("Error\n");
        return 0;
    }

    do
    {
        result = MainLoop(&deviceData);
    }
    while (result);

    UsbDeviceClose(&deviceData);
    return 0;
}
