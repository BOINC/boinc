using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;

namespace BoincBootstrap.Core.Configuration.Loaders
{
    public class XmlDeserializer<T> where T : class
    {
        private string path;

        public XmlDeserializer(string path)
        {
            this.path = path;
        }

        public T Deserialize()
        {
            T result;

            try
            {
                XmlSerializer xmlSerializer = new XmlSerializer(typeof(T));
                using (FileStream fileStream = new FileStream(this.path, FileMode.Open))
                {
                    XmlReader xmlReader = XmlReader.Create(fileStream);
                    result = (T)xmlSerializer.Deserialize(xmlReader);
                }
            }
            catch (Exception)
            {
                result = null;
            }

            return result;
        }
    }
}
