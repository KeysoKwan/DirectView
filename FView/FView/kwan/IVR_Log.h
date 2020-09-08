/*
简单的日志记录类. (使用spdlog内核）
KeysoKwan 2019.10.11

说明:(IvrLog.h)
1, 简单的单件实现（自动垃圾回收）
2, 使用方法：IvrLog::Inst()->Log("Run...",0);
*/
#pragma once
#ifndef EASY_LOG_H_8080
#    define EASY_LOG_H_8080
#    include <memory>
#    include <tchar.h>

#    include "spdlog/spdlog.h"
#    include "spdlog/logger.h"
#    include "spdlog/sinks/rotating_file_sink.h"

class IvrLog
{
  public:
    static IvrLog* Inst()
    {
        if (0 == _instance.get()) {
            CHAR szFilePath[MAX_PATH + 1] = {0};
            GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
            (strrchr(szFilePath, _T('\\')))[1] = 0; // 删除文件名，只获得路径字串
            std::string str_url(szFilePath);
            str_url.append(std::string("FAR_debug.log"));
            _instance.reset(new IvrLog);
            filelogger = spdlog::rotating_logger_mt("FARlog", str_url, 1048576 * 5, 1);
            spdlog::set_level(spdlog::level::trace);
            filelogger->flush_on(spdlog::level::level_enum::trace);
            filelogger->info("--------Log begin---------");
        }
        return _instance.get();
    }
    ///<summary>
    ///打印日志
    ///</summary>
    ///<param name="msg">需要打印的信息</param>
    ///<param name="_level">日志级别</param>
    void Log(std::string msg, int _level);
    void Log(TCHAR* msg, int _level);
    void Log(std::wstring loginfo, int _level);

    static std::string WString2String(const std::wstring& ws);

  private:
    IvrLog(void) {}
    virtual ~IvrLog(void);
    friend class std::auto_ptr<IvrLog>;
    static std::auto_ptr<IvrLog> _instance;
    static std::shared_ptr<spdlog::logger> filelogger;
};

//IvrLog::Inst()->Log("Run...");
#endif
