using System;
using Microsoft.Tools.WindowsInstallerXml.Bootstrapper;
using BoincBootstrap.Core.Enums;
using BoincBootstrap.Core.Packages;
using System.ComponentModel;
using BoincBootstrap.Core.View;
using System.Diagnostics;
using System.Threading;
using System.IO.Packaging;

namespace BoincBootstrap.Core
{
    public class BootstrapperController : IBootstrapperController
    {
        /// <summary>
        /// Fired when a critical error has been thrown with a message regarding what happened.
        /// </summary>
        public event Action<string> OnCriticalError;

        public IntPtr WindowHandle { get; set; }

        public IViewController ViewController { get; protected set; }

        public bool RestartRequired { get; protected set; }

        public BoincBootstrapper WixBootstrapper { get; protected set; }

        public int FinalResult { get; protected set; }

        public string Error { get; protected set; }

        public bool Cancelled { get; protected set; }

        public bool UpgradeDetected { get; protected set; }

        public InstallationResult InstallationResult { get; protected set; }

        public IPackageManager PackageManager { get; protected set; }

        public LaunchAction LaunchAction { get; protected set; }

        public bool Installed { get; protected set; }

        public int phaseCount { get; protected set; }

        /**
         * Events sequence:
         * // I=Install U=Uninstall L=Layout M=Modify R=Repair
            //// OnStartup (IULMR)
            ////  OnDetectBegin (IULMR)
            ////   // For each package
            ////   OnDetectPackageBegin (IULMR)
            ////   OnDetectPackageComplete (IULMR)
            ////  OnDetectComplete (IULMR)
            ////  OnPlanBegin (IULMR)
            ////   // For each package
            ////   OnPlanPackageBegin (IULMR)
            ////   OnPlanPackageComplete (IULMR)
            ////  OnPlanComplete (IULMR)
            ////  OnApplyPhaseCount (IULMR)
            ////  OnApplyBegin (IULMR)
            ////   OnCacheBegin (LR)
            ////    OnProgress (L)
            ////    OnCachePackageBegin (LR)
            ////     OnResolveSource (L)
            ////     OnCacheAquireBegin (LR)
            ////      OnCacheAcquireProgress (repeats) (LR)
            ////     OnCacheAquireComplete (LR)
            ////     OnCacheVerifyBegin (LR)
            ////      OnCacheAcquireProgress (repeats) (LR)
            ////     OnCacheVerifyComplete (LR)
            ////     OnProgress (LR)
            ////    OnCachePackageComplete (LR)
            ////   OnCacheComplete (LR)
            ////   OnElevate (IUM)
            ////   OnRegisterBegin (IUM)
            ////   OnRegisterComplete (IUM)
            ////   OnExecuteBegin (IULMR)
            ////    OnExecutePackageBegin (IUR)
            ////     OnExecuteProgress / OnExecuteMsiMessage (repeats) (IUR)
            ////     OnProgress (IUR)
            ////    OnExecutePackageComplete (IUR)
            ////   OnExecuteComplete (IULMR)
            ////   OnUnregisterBegin (IUMR)
            ////   OnUnregisterComplete (IUMR)
            ////  OnApplyComplete (IULMR)
            //// OnShutdown (IULMR)

            // Other events that may occur:
            // OnDetectCompatiblePackage
            // OnDetectForwardCompatibleBundle
            // OnDetectMsiFeature
            // OnDetectPriorBundle
            // OnDetectRelatedBundle
            // OnDetectRelatedMsiPackage
            // OnDetectTargetMsiPackage
            // OnDetectUpdate
            // OnDetectUpdateBegin
            // OnDetectUpdateComplete
            // OnError
            // OnExecuteFilesInUse
            // OnExecutePatchTarget
            // OnLaunchApprovedExeBegin
            // OnLaunchApprovedExeComplete
            // OnPlanCompatiblePackage
            // OnPlanMsiFeature
            // OnPlanRelatedBundle
            // OnPlanTargetMsiPackage
            // OnRestartRequired
            // OnSystemShutdown
         */

        public BootstrapperController(BoincBootstrapper wixBootstrapper, IViewController viewController, IPackageManager packageManager)
        {
            this.WixBootstrapper = wixBootstrapper;
            this.ViewController = viewController;
            this.PackageManager = packageManager;

            this.WixBootstrapper.DetectBegin += (sedner, eventArgs) => this.DetectBegin(eventArgs);
            this.WixBootstrapper.DetectComplete += (sedner, eventArgs) => this.DetectComplete(eventArgs);

            this.WixBootstrapper.PlanPackageComplete += (sedner, eventArgs) => this.PlanPackageComplete(eventArgs);
            this.WixBootstrapper.DetectRelatedBundle += (sedner, eventArgs) => this.DetectRelatedBundle(eventArgs);
            this.WixBootstrapper.DetectPackageComplete += (sedner, eventArgs) => this.DetectPackageComplete(eventArgs);

            this.WixBootstrapper.ApplyPhaseCount += (sender, eventArgs) => this.ApplyPhaseCount(eventArgs);

            this.WixBootstrapper.ApplyBegin += (sender, eventArgs) => this.ApplyBegin(eventArgs);
            this.WixBootstrapper.ExecutePackageBegin += (sender, eventArgs) => this.ExecutePackageBegin(eventArgs);
            this.WixBootstrapper.Progress += (sender, eventArgs) => this.Progress(eventArgs);
            this.WixBootstrapper.ExecutePackageComplete += (sender, eventArgs) => this.ExecutePackageComplete(eventArgs);
            this.WixBootstrapper.ApplyComplete += (sender, eventArgs) => this.ApplyComplete(eventArgs);

            this.WixBootstrapper.Shutdown += (sender, eventArgs) => this.Shutdown(eventArgs);
        }

        private void PlanPackageComplete(PlanPackageCompleteEventArgs eventArgs)
        {
            IPackage package = this.PackageManager.FindPackageById(eventArgs.PackageId);

            if (package != null)
            {
                package.RequestedState = eventArgs.Requested;
            }
        }

        private void ExecutePackageComplete(ExecutePackageCompleteEventArgs eventArgs)
        {
        }

        private void ExecutePackageBegin(ExecutePackageBeginEventArgs eventArgs)
        {
            IPackage package = this.PackageManager.FindPackageById(eventArgs.PackageId);
            if (!this.Installed)
            {
                this.WixBootstrapper.SetProgress(-1, "Installing " + package.ID);
            }
            else
            {
                if (this.UpgradeDetected)
                {
                    this.WixBootstrapper.SetProgress(-1, "Upgrading " + package.ID);
                }
                else
                {
                    this.WixBootstrapper.SetProgress(-1, "Removing " + package.ID);
                }
            }
        }

        private void Progress(ProgressEventArgs eventArgs)
        {
            this.WixBootstrapper.SetProgress(eventArgs.OverallPercentage, "");
        }

        private void ApplyPhaseCount(ApplyPhaseCountArgs eventArgs)
        {
            this.WixBootstrapper.SetProgress(5, "Unpacking installer...");
        }

        private void ApplyBegin(ApplyBeginEventArgs eventArgs)
        {
            this.WixBootstrapper.SetProgress(10, "");
        }

        private void ApplyComplete(ApplyCompleteEventArgs eventArgs)
        {
            this.WixBootstrapper.SetProgress(100, "Finishing");

            // Handle shutdown
            this.WixBootstrapper.Quit();
        }

        private void Shutdown(ShutdownEventArgs eventArgs)
        {
        }

        private void DetectPackageComplete(DetectPackageCompleteEventArgs eventArgs)
        {
            IPackage package = this.PackageManager.FindPackageById(eventArgs.PackageId);

            if (package != null)
            {
                package.PackageState = eventArgs.State;
            }
        }

        private void DetectRelatedBundle(DetectRelatedBundleEventArgs eventArgs)
        {
            if (eventArgs.Operation == RelatedOperation.MajorUpgrade || eventArgs.Operation == RelatedOperation.MinorUpdate)
            {
                this.UpgradeDetected = true;
            }
        }

        private void DetectBegin(DetectBeginEventArgs eventArgs)
        {
            this.Installed = eventArgs.Installed;
        }

        private void DetectComplete(DetectCompleteEventArgs eventArgs)
        {
            if (!this.Installed)
            {
                this.WixBootstrapper.SetProgress(-1, "Begin Installation");
                this.WixBootstrapper.Engine.Plan(LaunchAction.Install);
            }
            else
            {
                if (this.UpgradeDetected)
                {
                    this.WixBootstrapper.SetProgress(-1, "Begin Upgrade");
                    this.WixBootstrapper.Engine.Plan(LaunchAction.UpdateReplace);
                }
                else
                {
                    this.WixBootstrapper.SetProgress(-1, "Begin Uninstall");
                    this.WixBootstrapper.Engine.Plan(LaunchAction.Uninstall);
                }
            }

            this.WixBootstrapper.Engine.Apply(System.IntPtr.Zero);
        }
    }
}