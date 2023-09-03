using System.Windows;

namespace BoincBootstrap.Core.View.MainWindow
{
    public class BootstrapperMainWindowViewmodel : ObservableBase, IBootstrapperMainWindowViewmodel
    {
        public IBootstrapperController BootstrapperController { get; protected set; }

        public string Title { get; set; }

        public BootstrapperMainWindowViewmodel(IBootstrapperController bootstrapperController)
        {
            this.BootstrapperController = bootstrapperController;

            this.Title = this.BootstrapperController.WixBootstrapper.Engine.StringVariables["WixBundleName"];
        }

        public FrameworkElement CurrentView
        {
            get
            {
                return this.BootstrapperController.ViewController.CurrentViewmodel.View as FrameworkElement;
            }
        }
    }
}
