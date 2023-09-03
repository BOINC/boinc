using System;
using System.Xml.Serialization;

namespace BoincBootstrap.Core.Enums
{
    public class FeatureEnums
    {
        [Flags]
        public enum Attributes
        {
            FavorLocal = 0,
            FavorSource = 1,
            FollowParent = 2,
            FavorAdvertise = 4,
            DisallowAdvertise = 8,
            UIDisallowAbsent = 16,
            NoUnsupportedAdvertise = 32
        };

        public enum Display
        {
            [XmlEnum("0")]
            Hidden = 0,

            [XmlEnum("1")]
            Expanded = 1,

            [XmlEnum("2")]
            Collasped = 2
        }
    }
}
