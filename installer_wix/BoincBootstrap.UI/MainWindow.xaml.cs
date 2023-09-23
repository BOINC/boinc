namespace BoincBootstrap.UI
{
    using BoincBootstrap.Core;
    using BoincBootstrap.Core.View.MainWindow;
    using System.Reflection;
    using System.Threading;

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : IBootstrapperMainWindow
    {
        private readonly IBootstrapperController bootstrapperController;

        public MainWindow(IBootstrapperController bootstrapperController)
        {
            this.bootstrapperController = bootstrapperController;

            this.Viewmodel = new BootstrapperMainWindowViewmodel(bootstrapperController);

            this.InitializeComponent();
        }

        public IBootstrapperMainWindowViewmodel Viewmodel
        {
            get
            {
                return (IBootstrapperMainWindowViewmodel)DataContext;
            }

            set
            {
                this.DataContext = value;
            }
        }
    }
}