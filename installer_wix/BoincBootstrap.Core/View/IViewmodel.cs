namespace BoincBootstrap.Core.View
{
    public interface IViewmodel
    {
        IView View { get; set; }

        IBootstrapperController BootstrapperController { get; }

        void OnNavigatedTo();

        void OnNavigatedFrom();
    }
}