using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using xuexue;

namespace FViewTool
{
    /// <summary>
    /// App.xaml 的交互逻辑
    /// </summary>
    public partial class App : Application
    {
        private void Application_Startup(object sender, StartupEventArgs e)
        {
            //初始化日志文件在 %APPDATA%/FViewTool/log 文件夹
            DLog.dlog_init("FViewTool\\log", "FViewTool", DLog.INIT_RELATIVE.APPDATA, false);
            //设置立即flush的等级为最低
            DLog.dlog_set_flush_on(DLog.LOG_THR.debug);
            //输出一条日志,立即flush了
            DLog.LogI("App Start!");
        }
    }
}
