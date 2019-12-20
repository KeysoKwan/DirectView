#include "IVR_Log.h"
#include <Windows.h>

std::auto_ptr<IvrLog> IvrLog::_instance;
std::shared_ptr<spdlog::logger> IvrLog::filelogger;

void IvrLog::Log(std::string loginfo, int _level)
{
    switch (_level) {
    case ((int)spdlog::level::level_enum::trace):
        filelogger->trace(loginfo);
        break;
    case ((int)spdlog::level::level_enum::debug):
        filelogger->debug(loginfo);
        break;
    case ((int)spdlog::level::level_enum::info):
        filelogger->info(loginfo);
        break;
    case ((int)spdlog::level::level_enum::warn):
        filelogger->warn(loginfo);
        break;
    case ((int)spdlog::level::level_enum::err):
        filelogger->error(loginfo);
        break;
    case ((int)spdlog::level::level_enum::critical):
        filelogger->critical(loginfo);
        break;
    default:
        break;
    }
}

void IvrLog::Log(TCHAR* loginfo, int _level)
{
    int iLen = WideCharToMultiByte(CP_ACP, 0, loginfo, -1, NULL, 0, NULL, NULL);
    char* chRtn = new char[iLen * sizeof(char)];
    WideCharToMultiByte(CP_ACP, 0, loginfo, -1, chRtn, iLen, NULL, NULL);
    std::string str(chRtn);
    switch (_level) {
    case ((int)spdlog::level::level_enum::trace):
        filelogger->trace(str);
        break;
    case ((int)spdlog::level::level_enum::debug):
        filelogger->debug(str);
        break;
    case ((int)spdlog::level::level_enum::info):
        filelogger->info(str);
        break;
    case ((int)spdlog::level::level_enum::warn):
        filelogger->warn(str);
        break;
    case ((int)spdlog::level::level_enum::err):
        filelogger->error(str);
        break;
    case ((int)spdlog::level::level_enum::critical):
        filelogger->critical(str);
        break;
    default:
        break;
    }
}

void IvrLog::Log(std::wstring loginfo, int _level)
{
    std::string str = WString2String(loginfo);
    switch (_level) {
    case ((int)spdlog::level::level_enum::trace):
        filelogger->trace(str);
        break;
    case ((int)spdlog::level::level_enum::debug):
        filelogger->debug(str);
        break;
    case ((int)spdlog::level::level_enum::info):
        filelogger->info(str);
        break;
    case ((int)spdlog::level::level_enum::warn):
        filelogger->warn(str);
        break;
    case ((int)spdlog::level::level_enum::err):
        filelogger->error(str);
        break;
    case ((int)spdlog::level::level_enum::critical):
        filelogger->critical(str);
        break;
    default:
        break;
    }
}

std::string IvrLog::WString2String(const std::wstring& ws)
{
    size_t tempsize = 0;
    std::string strLocale = setlocale(LC_ALL, "");
    const wchar_t* wchSrc = ws.c_str();
    char chDest[512];
    wcstombs_s(&tempsize, chDest, 512, wchSrc, wcslen(wchSrc));
    std::string strResult = chDest;
    setlocale(LC_ALL, strLocale.c_str());
    return strResult;
}

IvrLog::~IvrLog(void)
{
    if (0 != _instance.get()) {
        _instance.release();
    }
}
