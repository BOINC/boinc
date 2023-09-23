using BoincBootstrap.Core.Packages;
using System;
using BoincBootstrap.Core.Enums;
using Microsoft.Tools.WindowsInstallerXml.Bootstrapper;
using BoincBootstrap.Core.View;

namespace BoincBootstrap.Core
{
    public interface IBootstrapperController
    {
        event Action<string> OnCriticalError;

        IntPtr WindowHandle { get; set;  }

        int FinalResult { get; }

        string Error { get; }

        bool Cancelled { get; }

        bool Installed { get; }

        bool UpgradeDetected { get; }

        LaunchAction LaunchAction { get; }

        InstallationResult InstallationResult { get; }

        bool RestartRequired { get; }

        BoincBootstrapper WixBootstrapper { get; }

        IViewController ViewController { get; }

        IPackageManager PackageManager { get; }
    }
}
