using System.Windows;

namespace BoincBootstrap.Core.View.MainWindow
{
    public interface IBootstrapperMainWindowViewmodel
    {
        IBootstrapperController BootstrapperController { get; }

        FrameworkElement CurrentView { get; }
    }
}
