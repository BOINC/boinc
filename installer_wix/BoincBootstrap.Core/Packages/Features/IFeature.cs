using System.Collections.Generic;
using BoincBootstrap.Core.Enums;

namespace BoincBootstrap.Core.Packages.Features
{
    public interface IFeature
    {
        int SubFeatureCount { get; }

        string FeatureId { get; }

        uint Size { get; }

        string Title { get; }

        string Description { get; }

        FeatureEnums.Display Display { get; }

        IEnumerable<IFeature> SubFeatures { get; }

        void AddSubFeature(IFeature feature);

        IFeature SearchSubFeatures(string featureId);
    }
}
