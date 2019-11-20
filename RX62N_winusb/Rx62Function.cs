using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO; //MemoryStream
using Microsoft.Win32.SafeHandles;

namespace RX62N_winusb
{
    internal class RX62Function
    {
        private const String USB_GUID_STRING = "{94A23B10-6D99-4903-AD6E-4042D723E4AB}";
        private DeviceManagement myDeviceManagement = new DeviceManagement();
        private String myDevicePathName;
        private UsbDevice myUsbDevice = new UsbDevice();

        private delegate void ReadFromDeviceDelegate
            (Byte pipeID,
            UInt32 bufferLength,
            ref Byte[] buffer,
            ref UInt32 lengthTransferred,
            ref Boolean success);

        public RX62Function()
        {
            Boolean deviceFound;
            String devicePathName = "";
            Boolean success;

            try
            {
                System.Guid UsbGuid = new System.Guid(USB_GUID_STRING);
                deviceFound = myDeviceManagement.FindDeviceFromGuid
                    (UsbGuid,
                    ref devicePathName);

                if (!deviceFound)
                {
                    Console.WriteLine("Device GUID is not matched:" + USB_GUID_STRING);
                    return;
                }

                success = myUsbDevice.GetDeviceHandle(devicePathName);
                if (success)
                {
                    Console.WriteLine("Device detected:" + devicePathName);
                    myDevicePathName = devicePathName;
                }
                else
                {
                    Console.WriteLine("Device is not detected:" + devicePathName);
                    myUsbDevice.CloseDeviceHandle();
                    return;
                }

                success = myUsbDevice.InitializeDevice();
                if (!success)
                {
                    Console.WriteLine("Device is not detected:" + devicePathName);

                    myUsbDevice.CloseDeviceHandle();

                    return;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                throw;
            }
        }

        ~RX62Function()
        {
            this.Close();
        }

        public void Close()
        {
            myUsbDevice.CloseDeviceHandle();
        }

        public int GetDipSW()
        {
            int sw;
            int count;
            byte[] buffer = new byte[1];

            count = (int) myUsbDevice.Read(ref buffer, 1);
            sw = (int) buffer[0];
            return sw;
        }

        public bool SetLED(int pattern)
        {
            int count;
            byte[] buffer = new byte[1];
            
            buffer[0] = (byte) pattern;
            count = (int) myUsbDevice.Write(ref buffer, 1);
            return (count == 1);
        }

        public bool GetIntreruptSW(byte[] buffer)
        {
            int count = (int) myUsbDevice.InterruptIn(ref buffer, 1);
            return (count == 1);
        }
    }

    sealed internal partial class UsbDevice
    {
        internal struct devInfo
        {
            internal SafeFileHandle deviceHandle;
            internal IntPtr winUsbHandle;
            internal Byte bulkInPipe;
            internal Byte bulkOutPipe;
            internal Byte interruptInPipe;
        }
        internal devInfo myDevInfo = new devInfo();

        ///  <summary>
        ///  Closes the device handle obtained with CreateFile and frees resources.
        ///  </summary>
        ///  
        internal void CloseDeviceHandle()
        {
            try
            {
                if (!(myDevInfo.winUsbHandle.Equals(IntPtr.Zero)))
                {
                    WinUsb_Free(myDevInfo.winUsbHandle);
                    myDevInfo.winUsbHandle = IntPtr.Zero;

                    if (!(myDevInfo.deviceHandle.IsInvalid))
                    {
                        myDevInfo.deviceHandle.Close();
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                throw;
            }
        }

        ///  <summary>
        ///  Requests a handle with CreateFile.
        ///  </summary>
        ///  
        ///  <param name="devicePathName"> Returned by SetupDiGetDeviceInterfaceDetail 
        ///  in an SP_DEVICE_INTERFACE_DETAIL_DATA structure. </param>
        ///  
        ///  <returns>
        ///  The handle.
        ///  </returns>

        internal Boolean GetDeviceHandle(String devicePathName)
        {
            // ***
            // API function

            //  summary
            //  Retrieves a handle to a device.

            //  parameters 
            //  Device path name returned by SetupDiGetDeviceInterfaceDetail
            //  Type of access requested (read/write).
            //  FILE_SHARE attributes to allow other processes to access the device while this handle is open.
            //  Security structure. Using Null for this may cause problems under Windows XP.
            //  Creation disposition value. Use OPEN_EXISTING for devices.
            //  Flags and attributes for files. The winsub driver requires FILE_FLAG_OVERLAPPED.
            //  Handle to a template file. Not used.

            //  Returns
            //  A handle or INVALID_HANDLE_VALUE.
            // ***

            myDevInfo.deviceHandle = FileIO.CreateFile
                (devicePathName,
                (FileIO.GENERIC_WRITE | FileIO.GENERIC_READ),
                FileIO.FILE_SHARE_READ | FileIO.FILE_SHARE_WRITE,
                IntPtr.Zero,
                FileIO.OPEN_EXISTING,
                FileIO.FILE_ATTRIBUTE_NORMAL | FileIO.FILE_FLAG_OVERLAPPED,
                0);

            if (!(myDevInfo.deviceHandle.IsInvalid))
            {
                return true;
            }
            else
            {
                return false;
            }
        }


        ///  <summary>
        ///  Initializes a device interface and obtains information about it.
        ///  Calls these winusb API functions:
        ///    WinUsb_Initialize
        ///    WinUsb_QueryInterfaceSettings
        ///    WinUsb_QueryPipe
        ///  </summary>
        ///  
        ///  <param name="deviceHandle"> A handle obtained in a call to winusb_initialize. </param>
        ///  
        ///  <returns>
        ///  True on success, False on failure.
        ///  </returns>

        internal Boolean InitializeDevice()
        {
            USB_INTERFACE_DESCRIPTOR ifaceDescriptor;
            WINUSB_PIPE_INFORMATION pipeInfo;
            //UInt32 pipeTimeout = 2000;
            Boolean success;
            //IntPtr[] winUsbHandles = new IntPtr[2];
            //IntPtr winUsbHandles = new IntPtr();
            IntPtr winUsbHandle;

            try
            {
                ifaceDescriptor.bLength = 0;
                ifaceDescriptor.bDescriptorType = 0;
                ifaceDescriptor.bInterfaceNumber = 0;
                ifaceDescriptor.bAlternateSetting = 0;
                ifaceDescriptor.bNumEndpoints = 0;
                ifaceDescriptor.bInterfaceClass = 0;
                ifaceDescriptor.bInterfaceSubClass = 0;
                ifaceDescriptor.bInterfaceProtocol = 0;
                ifaceDescriptor.iInterface = 0;

                pipeInfo.PipeType = 0;
                pipeInfo.PipeId = 0;
                pipeInfo.MaximumPacketSize = 0;
                pipeInfo.Interval = 0;

                success = WinUsb_Initialize
                    (myDevInfo.deviceHandle,
                    ref myDevInfo.winUsbHandle);
                if (!success)
                {
                    Console.WriteLine("Failed to initialize WinUSB.");
                    return false;
                }

                winUsbHandle = myDevInfo.winUsbHandle;

                success = WinUsb_QueryInterfaceSettings
                        (winUsbHandle,
                        0,
                        ref ifaceDescriptor);

                if (!success)
                {
                    Console.WriteLine("Failed to QueryInterfaceSettings.");
                    return false;
                }

                for (Int32 i = 0; i <= ifaceDescriptor.bNumEndpoints - 1; i++)
                {
                    success = WinUsb_QueryPipe
                            (winUsbHandle,
                            0,
                            System.Convert.ToByte(i),
                            ref pipeInfo);

                    if (((pipeInfo.PipeType ==
                        USBD_PIPE_TYPE.UsbdPipeTypeBulk) &
                        UsbEndpointDirectionIn(pipeInfo.PipeId)))
                    {
                        myDevInfo.bulkInPipe = pipeInfo.PipeId;
                    }
                    else if (((pipeInfo.PipeType ==
                        USBD_PIPE_TYPE.UsbdPipeTypeBulk) &
                        UsbEndpointDirectionOut(pipeInfo.PipeId)))
                    {
                        myDevInfo.bulkOutPipe = pipeInfo.PipeId;
                    }
                    else if ((pipeInfo.PipeType ==
                        USBD_PIPE_TYPE.UsbdPipeTypeInterrupt) &
                        UsbEndpointDirectionIn(pipeInfo.PipeId))
                    {
                        myDevInfo.interruptInPipe = pipeInfo.PipeId;
                    }
                    else
                    {
                        success = false;
                    }
                }

                if (!success || myDevInfo.bulkInPipe == 0
                             || myDevInfo.bulkOutPipe == 0
                             || myDevInfo.interruptInPipe == 0)
                {
                    Console.WriteLine("Failed to get pipe. Error");
                    return false;
                }

                WinUsb_ResetPipe(
                    myDevInfo.winUsbHandle,
                    myDevInfo.interruptInPipe
                );

                WinUsb_ResetPipe(
                    myDevInfo.winUsbHandle,
                    myDevInfo.bulkInPipe
                );

                WinUsb_ResetPipe(
                    myDevInfo.winUsbHandle,
                    myDevInfo.bulkOutPipe
                );

                return success;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                throw;
            }
        }

        internal UInt32 Read(ref Byte[] buffer, UInt32 bytesToRead)
        {
            UInt32 bytesRead = 0;

            if (!ReadViaBulkTransfer(bytesToRead, ref buffer, ref bytesRead))
            {
                bytesRead = 0;
            }
            return bytesRead;
        }

        internal UInt32 Write(ref Byte[] buffer, UInt32 bytesToWrite)
        {
            if (!SendViaBulkTransfer(ref buffer, bytesToWrite))
            {
                bytesToWrite = 0;
            }
            return bytesToWrite;
        }

        internal UInt32 InterruptIn(ref Byte[] buffer, UInt32 bytesToRead)
        {
            UInt32 bytesRead = 0;

            if (!ReadViaInterruptTransfer(bytesToRead, ref buffer, ref bytesRead))
            {
                bytesRead = 0;
            }
            return bytesRead;
        }

        ///  <summary>
		///  Attempts to read data from a bulk IN endpoint.
		///  </summary>
		///  
		///  <param name="InterfaceHandle"> Device interface handle. </param>
		///  <param name="PipeID"> Endpoint address. </param>
		///  <param name="bytesToRead"> Number of bytes to read. </param>
		///  <param name="Buffer"> Buffer for storing the bytes read. </param>
		///  <param name="bytesRead"> Number of bytes read. </param>
		///  <param name="success"> Success or failure status. </param>
		///  
		internal bool ReadViaBulkTransfer(UInt32 bytesToRead, ref Byte[] buffer, ref UInt32 bytesRead)
		{
            bool success;
			try
			{
				// ***
				//  winusb function 

				//  summary
				//  Attempts to read data from a device interface.

				//  parameters
				//  Device handle returned by WinUsb_Initialize.
				//  Endpoint address.
				//  Buffer to store the data.
				//  Maximum number of bytes to return.
				//  Number of bytes read.
				//  Null pointer for non-overlapped.

				//  Returns
				//  True on success.
				// ***

				success = WinUsb_ReadPipe
                    (myDevInfo.winUsbHandle,
                    myDevInfo.bulkInPipe,
					buffer,
					bytesToRead,
					ref bytesRead,
					IntPtr.Zero);

				if (!(success))
				{
					CloseDeviceHandle();
				}
			}
			catch (Exception ex)
			{
                Console.WriteLine(ex.Message); 
                throw;
			}
            return success;
		}

		///  <summary>
		///  Attempts to send data via a bulk OUT endpoint.
		///  </summary>
		///  
		///  <param name="buffer"> Buffer containing the bytes to write. </param>
		///  <param name="bytesToWrite"> Number of bytes to write. </param>
		///  
		///  <returns>
		///  True on success, False on failure.
		///  </returns>

		internal Boolean SendViaBulkTransfer(ref Byte[] buffer, UInt32 bytesToWrite)
		{
			UInt32 bytesWritten = 0;
			Boolean success;

			try
			{
				// ***
				//  winusb function 

				//  summary
				//  Attempts to write data to a device interface.

				//  parameters
				//  Device handle returned by WinUsb_Initialize.
				//  Endpoint address.
				//  Buffer with data to write.
				//  Number of bytes to write.
				//  Number of bytes written.
				//  IntPtr.Zero for non-overlapped I/O.

				//  Returns
				//  True on success.
				//  ***

				success = WinUsb_WritePipe
                    (myDevInfo.winUsbHandle,
					myDevInfo.bulkOutPipe,
					buffer,
					bytesToWrite,
					ref bytesWritten,
					IntPtr.Zero);

				if (!success)
				{
					CloseDeviceHandle();
				}
				return success;
			}
			catch (Exception ex)
			{
                Console.WriteLine(ex.Message);
				throw;
			}
		}

        internal bool ReadViaInterruptTransfer(UInt32 bytesToRead, ref Byte[] buffer, ref UInt32 bytesRead)
        {
            bool success;
            try
            {
                // ***
                //  winusb function 

                //  summary
                //  Attempts to read data from a device interface.

                //  parameters
                //  Device handle returned by WinUsb_Initialize.
                //  Endpoint address.
                //  Buffer to store the data.
                //  Maximum number of bytes to return.
                //  Number of bytes read.
                //  Null pointer for non-overlapped.

                //  Returns
                //  True on success.
                // ***

                success = WinUsb_ReadPipe
                    (myDevInfo.winUsbHandle,
                    myDevInfo.interruptInPipe,
                    buffer,
                    bytesToRead,
                    ref bytesRead,
                    IntPtr.Zero);

                if (!(success))
                {
                    CloseDeviceHandle();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                throw;
            }
            return success;
        }

		///  <summary>
		///  Is the endpoint's direction IN (device to host)?
		///  </summary>
		///  
		///  <param name="addr"> The endpoint address. </param>
		///  <returns>
		///  True if IN (device to host), False if OUT (host to device)
		///  </returns> 

		private Boolean UsbEndpointDirectionIn(Int32 addr)
		{
			Boolean directionIn;

			try
			{
				if (((addr & 0X80) == 0X80))
				{
					directionIn = true;
				}
				else
				{
					directionIn = false;
				}

			}
			catch (Exception ex)
			{
                Console.WriteLine(ex.Message); 
                throw;
			}
			return directionIn;
		}


		///  <summary>
		///  Is the endpoint's direction OUT (host to device)?
		///  </summary>
		///  
		///  <param name="addr"> The endpoint address. </param>
		///  
		///  <returns>
		///  True if OUT (host to device, False if IN (device to host)
		///  </returns>

		private Boolean UsbEndpointDirectionOut(Int32 addr)
		{
			Boolean directionOut;

			try
			{
				if (((addr & 0X80) == 0))
				{
					directionOut = true;
				}
				else
				{
					directionOut = false;
				}
			}
			catch (Exception ex)
			{
                Console.WriteLine(ex.Message); 
                throw;
			}
			return directionOut;
		}
    }
}
