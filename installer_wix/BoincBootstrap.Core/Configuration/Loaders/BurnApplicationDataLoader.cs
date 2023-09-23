using System.IO;
using System.Reflection;

namespace BoincBootstrap.Core.Configuration.Loaders
{
    internal class BurnApplicationDataLoader
    {
        private const string BootstrapperApplicationDataFilename = "BootstrapperApplicationData.xml";

        private XmlDeserializer<BurnApplicationInfo> deserializer;

        public BurnApplicationDataLoader()
        {
            var pathToTempInstallationDirectory = Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), BootstrapperApplicationDataFilename);
            this.deserializer = new XmlDeserializer<BurnApplicationInfo>(pathToTempInstallationDirectory);
        }

        public BurnApplicationInfo Load()
        {
            return this.deserializer.Deserialize();
        }
    }
}
