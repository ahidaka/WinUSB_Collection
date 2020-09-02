#include "buffer.h"

static PUSB_BUFFER UsbRxBuffer;

BOOL BufferInitialize(VOID)
{
    PUSB_BUFFER pu;

    pu = (PUSB_BUFFER)LocalAlloc(LPTR, sizeof(USB_BUFFER));
    if (pu == NULL)
    {
        return FALSE;
    }

    ZeroMemory(pu, sizeof(USB_BUFFER));
    UsbRxBuffer = pu;

    return TRUE;
}


BOOL BufferOutputTo(UCHAR Data)
{
    PUSB_BUFFER pu = UsbRxBuffer;

    if (pu->RxSystem == pu->RxUser)
    {
        if (pu->RxStatus == RX_OVER_FLOW)
        {
            return FALSE;
        }
    }
    pu->RxBuffer[pu->RxSystem] = Data;
    pu->RxSystem++;
    pu->RxSystem &= (RX_BUFSIZE - 1); /* rotete to top, if overlapped */

    if (pu->RxSystem == pu->RxUser)
    {
        pu->RxStatus = RX_OVER_FLOW;
    }
    else
    {
        pu->RxStatus = RX_RECEIVED;
    }
    pu->RxCount++;
    return TRUE;
}

BOOL BufferInputFrom(PUCHAR pData)
{
    PUSB_BUFFER pu = UsbRxBuffer;
    UCHAR data;
    BOOL status;

    if (pu->RxCount > 0)
    {
        data = pu->RxBuffer[pu->RxUser];
        pu->RxUser++;
        pu->RxUser &= (RX_BUFSIZE - 1); /* rotete to top, if overlapped */
        if (--pu->RxCount == 0)
        {
            pu->RxStatus = RX_EMPTY;
        }
        *pData = data;
        status = TRUE;
    }
    else
    {
        status = FALSE;
    }
    return status;
}


VOID BufferPrint(VOID)
{
    PUSB_BUFFER pu = UsbRxBuffer;
    ULONG i;
    CHAR* mark;

    if (pu->RxCount <= 0)
    {
        return;
    }

    //       0  1  2  3  4  5  6  7
    printf("RxStatus=%d RxCount=%2d", pu->RxStatus, pu->RxCount);

    for (i = 8; i < RX_BUFSIZE; i++)
    {
        printf("%2d ", i);
    }
    printf("\n");

    for (i = 0; i < RX_BUFSIZE; i++)
    {
        if (i == pu->RxSystem && i == pu->RxUser)
        {
            mark = "su";
        }
        else if (i == pu->RxSystem)
        {
            mark = "ss";
        }
        else if (i == pu->RxUser)
        {
            mark = "uu";
        }
        else {
            mark = "  ";
        }
        printf("%s ", mark);
    }
    printf("\n");

    for (i = 0; i < RX_BUFSIZE; i++)
    {
        printf("%02X ", pu->RxBuffer[i]);
    }
    printf("\n");
}

