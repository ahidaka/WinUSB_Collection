using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;
using System.Reflection;

namespace RX62N_winusb
{
    public partial class MainWindow : Window
    {
        internal Dictionary<int, int> buttons = new Dictionary<int, int>();
        internal RX62Function rx62n;
        internal int sevenSegment;
        internal bool working;
        System.Threading.Thread interruptThread;

        public MainWindow()
        {
            InitializeComponent();

            for (int i = 0; i < 8; i++)
            {
                Button b = GetObjectFromName(string.Format("sevenSegment{0}", i)) as Button;
                b.ClearValue(Button.BackgroundProperty);
                buttons[b.GetHashCode()] = i;
            }

            rx62n = new RX62Function();
            int mask = 0x01;
            for (int i = 0; i < 8; i++)
            {
                rx62n.SetLED(mask);
                mask <<= 1;
                System.Threading.Thread.Sleep(100);
            }
            rx62n.SetLED(sevenSegment = 0);

            working = true;
            interruptThread = new System.Threading.Thread(InterruptThread);
            interruptThread.Start();
        }

        private Object GetObjectFromName(string fieldname)
        {
            Object o = null;
            FieldInfo fi = this.GetType().GetField(fieldname,
                System.Reflection.BindingFlags.Public |
                System.Reflection.BindingFlags.NonPublic |
                System.Reflection.BindingFlags.Instance |
                System.Reflection.BindingFlags.DeclaredOnly);

            if (fi != null)
            {
                o = fi.GetValue(this);
            }
            return o;
        }

        private void InterruptThread()
        {
            byte[] pattern = new byte[1];
            while (working)
            {
                bool swStatus = rx62n.GetIntreruptSW(pattern);
                if (pattern[0] != 0)
                {
                    Console.WriteLine("Switch ON\n");
                    Dispatcher.BeginInvoke(
                            new DispatcherOperationCallback(TextBlockText),
                            "Switch ON");
                }
                //////////////////////////////////////////////
                // For Driver INTERRUPT-IN data buffer test //
                // System.Threading.Thread.Sleep(2000);     //
                //////////////////////////////////////////////
                System.Threading.Thread.Sleep(100);
            }
        }

        object TextBlockText(object obj)
        {
            textBlock.Text = obj as string;
            buttonOK.Visibility = System.Windows.Visibility.Visible;
            return null;
        }

        private void ButtonSetLedClick(object sender, RoutedEventArgs e)
        {
            textBlock.Text = "buttonSetLed ";
            int mask = 0x01;

            for (int i = 0; i < 8; i++)
            {
                CheckBox c = GetObjectFromName(string.Format("LEDcheck{0}", i)) as CheckBox;
                Button b = GetObjectFromName(string.Format("sevenSegment{0}", i)) as Button;
                if ((bool)c.IsChecked)
                {
                    textBlock.Text += (i.ToString() + " ");
                    b.Background = Brushes.OrangeRed;
                    sevenSegment |= mask;
                }
                else
                {
                    b.ClearValue(Button.BackgroundProperty);
                    sevenSegment &= ~mask;
                }
                mask <<= 1;
            }
            rx62n.SetLED(sevenSegment);
        }

        private void ButtonGetDipSwClick(object sender, RoutedEventArgs e)
        {
            textBlock.Text = "buttonGetDipSW";
            int sw = rx62n.GetDipSW();
            int mask = 0x01;

            for (int i = 0; i < 8; i++)
            {
                CheckBox c = GetObjectFromName(string.Format("DIPcheck{0}", i)) as CheckBox;
                c.IsChecked = (sw & mask) != 0;
                mask <<= 1;
            }
        }

        private void ButtonOKClick(object sender, RoutedEventArgs e)
        {
            textBlock.Text = "OK Clicked";
            buttonOK.Visibility = System.Windows.Visibility.Hidden;
        }

        private void SevenSegmentCheck(int i)
        {
            int mask = 0x01 << i;
            Boolean oldStatusIsON = (sevenSegment & mask) != 0;

            if (oldStatusIsON)
            {
                sevenSegment &= ~mask;
            }
            else
            {
                sevenSegment |= mask;
            }

            CheckBox c = GetObjectFromName(string.Format("LEDcheck{0}", i)) as CheckBox;
            Button b = GetObjectFromName(string.Format("sevenSegment{0}", i)) as Button;
            c.IsChecked = !(bool)c.IsChecked;
            if ((bool)c.IsChecked)
            {
                b.Background = Brushes.OrangeRed;
            }
            else
            {
                b.ClearValue(Button.BackgroundProperty);
            }
            rx62n.SetLED(sevenSegment);
        }

        private void SevenSegmentClick(object sender, RoutedEventArgs e)
        {
            SevenSegmentCheck(buttons[sender.GetHashCode()]);
        }

        private void ClearClick(object sender, RoutedEventArgs e)
        {
            for (int i = 0; i < 8; i++)
            {
                CheckBox c = GetObjectFromName(string.Format("LEDcheck{0}", i)) as CheckBox;
                Button b = GetObjectFromName(string.Format("sevenSegment{0}", i)) as Button;
                c.IsChecked = false;
                b.ClearValue(Button.BackgroundProperty);
            }
            rx62n.SetLED(sevenSegment = 0);
            textBlock.Text = "LED cleared";
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            working = false;
            if (rx62n != null)
            {
                rx62n.SetLED(sevenSegment = 0);
                System.Threading.Thread.Sleep(350);
                rx62n.Close();
            }
        }
    }

}
