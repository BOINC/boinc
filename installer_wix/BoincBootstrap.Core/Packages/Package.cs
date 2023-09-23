using BoincBootstrap.Core.Packages.Features;
using System.Collections.Generic;
using BoincBootstrap.Core.Enums;
using Microsoft.Tools.WindowsInstallerXml.Bootstrapper;

namespace BoincBootstrap.Core.Packages
{
    public class Package : ObservableBase, IPackage
    {
        private PackageState packageState;

        private RequestState requestedState;

        private IList<IFeature> features;

        public string ID { get; protected set; }

        public string DisplayName { get; protected set; }

        public Package(string id, string displayName)
        {
            this.ID = id;
            this.DisplayName = displayName;
            this.features = new List<IFeature>();
        }

        public IEnumerable<IFeature> Features
        {
            get
            {
                return this.features;
            }
        }

        public PackageState PackageState
        {
            get
            {
                return this.packageState;
            }

            set
            {
                this.packageState = value;
            }
        }

        public RequestState RequestedState
        {
            get
            {
                return this.requestedState;
            }

            set
            {
                this.requestedState = value;
            }
        }

        public void AddFeature(IFeature feature)
        {
            this.features.Add(feature);
        }

        public IFeature SearchFeature(string featureId)
        {
            IFeature foundFeature = null;

            foreach (IFeature feature in this.features)
            {
                if (feature.FeatureId.Equals(featureId))
                {
                    foundFeature = feature;
                }
                else
                {
                    foundFeature = feature.SearchSubFeatures(featureId);
                }

                if (foundFeature != null)
                {
                    break;
                }
            }

            return foundFeature;
        }
    }
}
