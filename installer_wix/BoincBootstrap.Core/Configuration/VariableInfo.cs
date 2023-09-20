using System.Xml.Serialization;

namespace BoincBootstrap.Core.Configuration
{
    public class VariableInfo
    {
        [XmlAttribute("Caption")]
        public string Caption { get; private set; }

        [XmlAttribute("Name")]
        public string Name { get; private set; }
    }
}
