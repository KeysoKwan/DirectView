using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;
using xuexue;

namespace FViewTool
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            this.ocv_image.Source = writableImage;

            _pathParamC920 = System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "param", "C920.yml");

            //这个函数以管理员模式运行程序也能找到appdata文件夹没什么问题
            _savePath = System.IO.Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "FViewTool", "FView.json");
            System.IO.FileInfo fi = new System.IO.FileInfo(_savePath);
            if (!System.IO.Directory.Exists(fi.Directory.FullName)) {
                System.IO.Directory.CreateDirectory(fi.Directory.FullName);
            }

            _timer = new System.Threading.Timer(OnTimer);
            _timer.Change(1000, 30);

            buff = writableImage.BackBuffer;
            DLog.LogI("savePath=" + _savePath);
            Task.Run(() => {
                //fview_start(_pathParamC920, "HD Pro Webcam C920", 1280, 720, buff);
                fview_start2(_pathParamC920, 1280, 720, buff);
            });

        }

        IntPtr buff;

        /// <summary>
        /// 参数文件路径
        /// </summary>
        string _pathParamC920;

        /// <summary>
        /// 保存参数文件位置
        /// </summary>
        string _savePath;

        /// <summary>
        /// 方块窗口
        /// </summary>
        WinCalc win = null;

        /// <summary>
        /// c++的opencv的图片
        /// </summary>
        WriteableBitmap writableImage = new WriteableBitmap(1280, 720, 96, 96, PixelFormats.Bgr24, null);

        /// <summary>
        /// 定时更新图像
        /// </summary>
        System.Threading.Timer _timer;


        private void Btn_openwin_Click(object sender, RoutedEventArgs e)
        {
            win = new WinCalc();
            win.Show();
            gcARTool.WindowPos.UpdateWindowPos(this, win);
        }

        private void Btn_ok_Click(object sender, RoutedEventArgs e)
        {
            bool isOK = fview_save(_savePath);
            if (isOK) {
                MessageBox.Show("保存成功!");
            }
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            _timer.Dispose();
            if (win != null) {
                win.Close();
            }
            Fun2();
        }

        private void Btn_closewin_Click(object sender, RoutedEventArgs e)
        {
            if (win != null) {
                win.Close();
            }
        }


        private void OnTimer(object state)
        {
            App.Current.Dispatcher.BeginInvoke(DispatcherPriority.Send, new Action(() => {
                UpdateWritableImg(writableImage);
            }));
        }

        /// <summary>
        /// 传递自己的BackBuffer去给opencv设置
        /// </summary>
        /// <param name="writableImg"></param>
        private void UpdateWritableImg(WriteableBitmap writableImg)
        {
            try {

                writableImage.Lock();
                writableImg.AddDirtyRect(new Int32Rect(0, 0, writableImg.PixelWidth, writableImg.PixelHeight));
                writableImg.Unlock();
            }
            catch (Exception) {
            }
        }

        /// <summary>
        /// 打开相机
        /// </summary>
        /// <returns></returns>
        [DllImport(@"F3DSystem.dll")]
        public static extern void fview_start(string camParamPath, string camName, int cam_w, int cam_h, IntPtr bmp);

        [DllImport(@"F3DSystem.dll")]
        public static extern void fview_start2(string camParamPath, int cam_w, int cam_h, IntPtr bmp);

        [DllImport(@"F3DSystem.dll")]
        public static extern bool fview_save(string savePath);

        [DllImport(@"F3DSystem.dll")]
        public static extern void Fun2();
    }
}
