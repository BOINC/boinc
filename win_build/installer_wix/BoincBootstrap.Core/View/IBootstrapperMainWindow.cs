using BoincBootstrap.Core.View;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BoincBootstrap.Core
{
    public interface IBootstrapperMainWindow
    {
        IBootstrapperMainWindowViewmodel Viewmodel { get; }
    }
}
