using System.Xml.Serialization;

namespace BoincBootstrap.Core.Configuration
{
    public class BundlePropertiesInfo
    {
        [XmlAttribute("DisplayName")]
        public string DisplayName { get; set; }
    }
}
