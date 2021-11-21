using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace ApplicationLib
{
    public class EntryPoint
    {
        private static Action hostCallback;

        // CLR restricts this method to accept only one string argument. We serialize native function pointer to string
        // and converted it back to address here
        public static int Main(string hostCallbackPtrString)
        {
            IntPtr hostCallbackPtr = new IntPtr(long.Parse(hostCallbackPtrString, NumberStyles.HexNumber));
            hostCallback = (Action)Marshal.GetDelegateForFunctionPointer(hostCallbackPtr, typeof(Action));

            WpfApplicationBootstrapper bootstrapper = new WpfApplicationBootstrapper(hostCallback);
            bootstrapper.Run();
            return 0;
        }
    }
}
