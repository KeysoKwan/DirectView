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
#    include <direct.h>
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
            std::string programName = strrchr(szFilePath, _T('\\'));
            std::string folderPath = "C:\\F3DFuture\\AppLog" + programName.erase(programName.find_last_of("."));
            if (0 != _access(folderPath.c_str(), 0)) {
                // if this folder not exist, create a new one.
                _mkdir(folderPath.c_str());
            }
            folderPath.append(std::string("\\FAR_debug.log"));
            _instance.reset(new IvrLog);
            filelogger = spdlog::rotating_logger_mt("FARlog", folderPath, 1048576 * 5, 1);
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
