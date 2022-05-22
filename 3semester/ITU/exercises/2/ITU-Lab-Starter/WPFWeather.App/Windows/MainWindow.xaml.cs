using System.Windows;
using WPFWeather.App.Services;
using WPFWeather.App.ViewModels;
using System;

namespace WPFWeather.App.Windows
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            WeatherViewModel vm = new WeatherViewModel(new WeatherDiskService());

            //vm.DownloadWeatherCommand.Execute("Brno");
            DataContext = vm;
        }
    }
}