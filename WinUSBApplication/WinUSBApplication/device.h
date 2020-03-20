//
// Define below GUIDs
//
#include <initguid.h>

//
// Device Interface GUID.
// Used by all WinUsb devices that this application talks to.
// Must match "DeviceInterfaceGUIDs" registry value specified in the INF file.
// b0fbcc5b-ed3a-491d-a6a3-5496193b7805
//
//DEFINE_GUID(GUID_DEVINTERFACE_WinUSBApplication,
//    0xb0fbcc5b,0xed3a,0x491d,0xa6,0xa3,0x54,0x96,0x19,0x3b,0x78,0x05);
DEFINE_GUID(GUID_DEVINTERFACE_WinUSBApplication,
    0xEA062151, 0xA777, 0x4E65, 0x8B, 0xE4, 0x24, 0xA4, 0xB5, 0x39, 0xFB, 0x42);

typedef struct _DEVICE_DATA {

    BOOL                    HandlesOpen;
    WINUSB_INTERFACE_HANDLE WinusbHandle;
    HANDLE                  DeviceHandle;
    TCHAR                   DevicePath[MAX_PATH];

} DEVICE_DATA, *PDEVICE_DATA;

HRESULT
OpenDevice(
    _Out_     PDEVICE_DATA DeviceData,
    _Out_opt_ PBOOL        FailureDeviceNotFound
    );

VOID
CloseDevice(
    _Inout_ PDEVICE_DATA DeviceData
    );
