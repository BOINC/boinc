using System.Collections.Generic;
using System.Linq;

namespace BoincBootstrap.Core.Packages
{
    public class PackageManager : IPackageManager
    {
        private IDictionary<string, IPackage> packageDictionary;

        public int PackageCount
        {
            get
            {
                return this.packageDictionary.Count();
            }
        }

        public IEnumerable<IPackage> Packages
        {
            get
            {
                return this.packageDictionary.Values;
            }
        }

        public PackageManager()
        {
            this.packageDictionary = new Dictionary<string, IPackage>();
        }

        public void AddPackage(IPackage package)
        {
            this.packageDictionary.Add(package.ID, package);
        }

        public IPackage FindPackageById(string id)
        {
            if (!string.IsNullOrEmpty(id) && this.packageDictionary.ContainsKey(id))
            {
                return this.packageDictionary[id];
            }

            return null;
        }
    }
}
