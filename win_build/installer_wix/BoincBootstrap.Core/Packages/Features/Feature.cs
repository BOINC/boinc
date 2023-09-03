using System;
using System.Collections.Generic;
using System.Linq;
using BoincBootstrap.Core.Enums;

using Display = BoincBootstrap.Core.Enums.FeatureEnums.Display;

namespace BoincBootstrap.Core.Packages.Features
{
    public class Feature : IFeature
    {
        private readonly IList<IFeature> subFeatures;

        public IEnumerable<IFeature> SubFeatures { get; }

        public string Description { get; protected set; }

        public Display Display { get; protected set; }

        public string FeatureId { get; protected set; }

        public Feature(string featureId, string title, string description, uint size, Display display)
        {
            this.FeatureId = featureId;
            this.Title = title;
            this.Description = description;
            this.Size = size;
            this.Display = display;

            this.subFeatures = new List<IFeature>();
        }

        public uint Size { get; protected set; }

        public string Title { get; protected set; }

        public int SubFeatureCount
        {
            get
            {
                return this.subFeatures.Count;
            }
        }

        public void AddSubFeature(IFeature newSubFeature)
        {
            var existingFeature = this.subFeatures.FirstOrDefault(f => f.FeatureId.Equals(newSubFeature.FeatureId));

            if (existingFeature == null)
            {
                this.subFeatures.Add(newSubFeature);
            }
        }

        public IFeature SearchSubFeatures(string featureId)
        {
            IFeature foundFeature = null;

            foreach (IFeature feature in this.SubFeatures)
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
