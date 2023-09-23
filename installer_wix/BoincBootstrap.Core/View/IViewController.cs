using BoincBootstrap.Core.Enums;
using System;

namespace BoincBootstrap.Core.View
{
    public interface IViewController
    {
        IViewmodel CurrentViewmodel { get; }

    }
}
