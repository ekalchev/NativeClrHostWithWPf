using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace ApplicationLib
{
    public partial class App : Application
    {
        private static Thread mainThread;
        public static event EventHandler MainWindowShown;

        public App()
        {
            ShutdownMode = ShutdownMode.OnMainWindowClose;
        }

        public void InitializeComponent()
        {
            StartupUri = new Uri("pack://application:,,,/ApplicationLib;component/MainWindow.xaml", UriKind.Absolute);
        }

        internal void NotifyMainWindowShown()
        {
            MainWindowShown.Invoke(this, EventArgs.Empty);
        }

        public static void _Run()
        {
            mainThread = new Thread(() =>
            {
                App app = new App();
                app.InitializeComponent();
                app.Run();
            });

            mainThread.SetApartmentState(ApartmentState.STA);
            mainThread.Start();
        }

        public static void WaitForExit()
        {
            mainThread.Join();
        }
    }
}
