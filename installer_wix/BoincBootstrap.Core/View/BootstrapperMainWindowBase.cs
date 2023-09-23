using System;
using System.Windows;
using BoincBootstrap.Core.View;

namespace BoincBootstrap.Core
{
    public abstract class BootstrapperMainWindowBase : Window, IBootstrapperMainWindow
    {
        public IBootstrapperMainWindowViewmodel Viewmodel
        {
            get
            {
                return (IBootstrapperMainWindowViewmodel)this.DataContext;
            }

            protected set
            {
                this.DataContext = value;
            }
        }
    }
}
