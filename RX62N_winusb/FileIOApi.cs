using System;
using Microsoft.Win32.SafeHandles;
using System.Runtime.InteropServices;

namespace RX62N_winusb
{
	sealed internal class FileIO
	{
		internal const Int32 FILE_ATTRIBUTE_NORMAL = 0X80;
		internal const Int32 FILE_FLAG_OVERLAPPED = 0X40000000;
		internal const Int32 FILE_SHARE_READ = 1;
		internal const Int32 FILE_SHARE_WRITE = 2;
		internal const UInt32 GENERIC_READ = 0X80000000;
		internal const UInt32 GENERIC_WRITE = 0X40000000;
		internal const Int32 INVALID_HANDLE_VALUE = -1;
		internal const Int32 OPEN_EXISTING = 3;

#if OLD_VER
        internal const UInt32 IOCTL_INDEX             = 0x800;
        internal const UInt32 FILE_DEVICE_RX62N       = 0x65500;

        internal const UInt32 METHOD_BUFFERED         = 0;
        internal const UInt32 METHOD_IN_DIRECT        = 1;
        internal const UInt32 METHOD_OUT_DIRECT       = 2;
        internal const UInt32 METHOD_NEITHER          = 3;

        internal const UInt32 FILE_ANY_ACCESS         = 0;
        internal const UInt32 FILE_SPECIAL_ACCESS     = (FILE_ANY_ACCESS);
        internal const UInt32 FILE_READ_ACCESS        = ( 0x0001 );    // file & pipe
        internal const UInt32 FILE_WRITE_ACCESS       = ( 0x0002 );    // file & pipe

        //#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
        //    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
        //)

        static Func<uint, uint, uint, uint, uint> ctlCode =
            (DeviceType, Function, Method, Access) =>
                ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method);

        internal static readonly
            UInt32 IOCTL_RX62N_GET_CONFIG_DESCRIPTOR = ctlCode(FILE_DEVICE_RX62N,
                                                     IOCTL_INDEX,
                                                     METHOD_BUFFERED,
                                                     FILE_READ_ACCESS);
                                     
        internal static readonly
            UInt32 IOCTL_RX62N_RESET_DEVICE = ctlCode(FILE_DEVICE_RX62N,
                                                     IOCTL_INDEX + 1,
                                                     METHOD_BUFFERED,
                                                     FILE_WRITE_ACCESS);

        internal static readonly
            UInt32 IOCTL_RX62N_REENUMERATE_DEVICE = ctlCode(FILE_DEVICE_RX62N,
                                                    IOCTL_INDEX  + 3,
                                                    METHOD_BUFFERED,
                                                    FILE_WRITE_ACCESS);

        internal static readonly
            UInt32 IOCTL_RX62N_READ_SWITCHES = ctlCode(FILE_DEVICE_RX62N,
                                                    IOCTL_INDEX + 6,
                                                    METHOD_BUFFERED,
                                                    FILE_READ_ACCESS);

        internal static readonly
            UInt32 IOCTL_RX62N_SET_7_SEGMENT_DISPLAY = ctlCode(FILE_DEVICE_RX62N,
                                                    IOCTL_INDEX + 8,
                                                    METHOD_BUFFERED,
                                                    FILE_WRITE_ACCESS);

        internal static readonly
            UInt32 IOCTL_RX62N_GET_INTERRUPT_MESSAGE = ctlCode(FILE_DEVICE_RX62N,
                                                    IOCTL_INDEX + 9,
                                                    METHOD_OUT_DIRECT,
                                                    FILE_READ_ACCESS);
#endif
		[DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
		internal static extern SafeFileHandle CreateFile(
            String lpFileName,
            UInt32 dwDesiredAccess,
            Int32 dwShareMode,
            IntPtr lpSecurityAttributes,
            Int32 dwCreationDisposition,
            Int32 dwFlagsAndAttributes,
            Int32 hTemplateFile);
#if OLD_VER
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        internal static extern bool DeviceIoControl(
            SafeFileHandle hDevice,
            uint dwIoControlCode,
            [MarshalAs(UnmanagedType.AsAny)]
            [In] object InBuffer,
            uint nInBufferSize,
            [MarshalAs(UnmanagedType.AsAny)]
            [Out] object OutBuffer,
            uint nOutBufferSize,
            ref int lpBytesReturned,
            int Overlapped);
#endif
    }
}
