using System.Collections.Generic;
using System.Xml.Serialization;

namespace BoincBootstrap.Core.Configuration
{
    public class ConfigurationInfo
    {
        [XmlAttribute("Caption")]
        public string Caption { get; set; }

        [XmlAttribute("Description")]
        public string Description { get; set; }

        [XmlAttribute("PackageIds")]
        public string PackageIds { get; set; }

        [XmlElement("WixVariable")]
        public List<VariableInfo> WixVariables { get; set; }
    }
}
