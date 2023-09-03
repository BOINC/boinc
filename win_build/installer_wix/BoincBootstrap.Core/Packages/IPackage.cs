using BoincBootstrap.Core.Packages.Features;
using Microsoft.Tools.WindowsInstallerXml.Bootstrapper;
using System.Collections.Generic;

namespace BoincBootstrap.Core.Packages
{
    public interface IPackage
    {
        PackageState PackageState { get; set; }

        RequestState RequestedState { get; set; }

        IEnumerable<IFeature> Features { get; }

        string ID { get; }

        string DisplayName { get; }

        void AddFeature(IFeature feature);

        IFeature SearchFeature(string featureId);
    }
}