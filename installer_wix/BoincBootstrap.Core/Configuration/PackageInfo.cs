using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace BoincBootstrap.Core.Configuration
{
    public class PackageInfo
    {
        [XmlAttribute("Package")]
        public string Id { get; set;}

        [XmlAttribute("DisplayName")]
        public string DisplayName { get; set; }

        [XmlAttribute("Description")]
        public string Description { get; set; }

        [XmlAttribute("Version")]
        public string Version { get; set; }

        [XmlAttribute("Vital")]
        public string Vital { get; set; }

        [XmlAttribute("ProductCode")]
        public Guid ProductCode { get; set; }

        [XmlAttribute("PackageType")]
        public string PackageType { get; set; }

        [XmlAttribute("Permanent")]
        public string Permanent { get; set; }

        [XmlAttribute("Cache")]
        public string Cache { get; set; }
    }
}
