using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace BoincBootstrap.Core.View
{
    public class BootstrapperMainWindowViewmodel : IBootstrapperMainWindowViewmodel
    {
        public IBootstrapperController BootstrapperController { get; protected set; }

        public BootstrapperMainWindowViewmodel(IBootstrapperController bootstrapperController)
        {
            this.BootstrapperController = bootstrapperController;
        }

        public FrameworkElement CurrentPage
        {
            get
            {
                return this.BootstrapperController.PageController.CurrentPage.ViewContent;
            }
        }
    }
}
