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
        private readonly Action hideSplashScreen;

        public App(Action hideSplashScreen)
        {
            ShutdownMode = ShutdownMode.OnMainWindowClose;
            this.hideSplashScreen = hideSplashScreen;
        }

        public void InitializeComponent()
        {
            StartupUri = new Uri("pack://application:,,,/ApplicationLib;component/MainWindow.xaml", UriKind.Absolute);
        }

        public void NotifyMainWindowShown()
        {
            hideSplashScreen();
        }
    }
}
