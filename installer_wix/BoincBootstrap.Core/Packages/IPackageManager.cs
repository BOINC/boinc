using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BoincBootstrap.Core.Packages
{
    public interface IPackageManager
    {
        IEnumerable<IPackage> Packages { get; }

        void AddPackage(IPackage package);

        IPackage FindPackageById(string id);

        int PackageCount { get; }
    }
}
