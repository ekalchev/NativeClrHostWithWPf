using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;

namespace ApplicationLib
{
    internal class WpfApplicationBootstrapper
    {
        private readonly Action hostCallBack;
        private Thread mainThread;

        public WpfApplicationBootstrapper(Action hostCallBack)
        {
            this.hostCallBack = hostCallBack;
        }

        public void Run()
        {
            mainThread = new Thread(() =>
            {
                App app = new App(HideSplashScreen);
                app.InitializeComponent();
                app.Run();
            });

            mainThread.SetApartmentState(ApartmentState.STA);
            mainThread.Start();
        }

        private void HideSplashScreen()
        {
            hostCallBack();
        }

        public void WaitForExit()
        {
            mainThread.Join();
        }
    }
}
