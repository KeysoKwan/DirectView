#include "IVR_Log.h"

#include <Windows.h>
std::auto_ptr<IvrLog> IvrLog::_instance;

void IvrLog::Log(std::string loginfo) {
#ifdef _DEBUG
	std::ofstream ofs;
	struct tm t;   //tm结构指针
	time_t now;  //声明time_t类型变量
	time(&now);      //获取系统日期和时间
	localtime_s(&t, &now);   //获取当地日期和时间
	char tmp[64];
	strftime(tmp, sizeof(tmp), "\t[%Y.%m.%d %X %A]", &t);
	ofs.open(filename, std::ofstream::app);
	ofs.write(loginfo.c_str(), loginfo.size());
	ofs << tmp << '\n';
	ofs.close();
#endif
}

void IvrLog::Log(TCHAR* loginfo) {
#ifdef _DEBUG
	int iLen = WideCharToMultiByte(CP_ACP, 0, loginfo, -1, NULL, 0, NULL, NULL);
	char* chRtn = new char[iLen * sizeof(char)];
	WideCharToMultiByte(CP_ACP, 0, loginfo, -1, chRtn, iLen, NULL, NULL);
	std::string str(chRtn);

	std::ofstream ofs;
	struct tm t;   //tm结构指针
	time_t now;  //声明time_t类型变量
	time(&now);      //获取系统日期和时间
	localtime_s(&t, &now);   //获取当地日期和时间
	char tmp[64]; 
	strftime(tmp, sizeof(tmp), "\t[%Y.%m.%d %X %A]", &t);
	ofs.open(filename, std::ofstream::app);
	ofs.write(str.c_str(), str.size());
	ofs << tmp << '\n';
	ofs.close();
	delete[] chRtn;
#endif
}

std::string WString2String(const std::wstring& ws)
{
    size_t tempsize = 0;
	std::string strLocale = setlocale(LC_ALL, "");
	const wchar_t* wchSrc = ws.c_str();
    char chDest[512];
    wcstombs_s(&tempsize, chDest,512, wchSrc, wcslen(wchSrc));
	std::string strResult = chDest;
	setlocale(LC_ALL, strLocale.c_str());
	return strResult;
}

void IvrLog::Log(std::wstring loginfo) {
#ifdef _DEBUG
	std::string str = WString2String(loginfo);
	std::ofstream ofs;
	struct tm t;   //tm结构指针
	time_t now;  //声明time_t类型变量
	time(&now);      //获取系统日期和时间
	localtime_s(&t, &now);   //获取当地日期和时间
	char tmp[64];
	strftime(tmp, sizeof(tmp), "\t[%Y.%m.%d %X %A]", &t);
	ofs.open(filename, std::ofstream::app);
	ofs.write(str.c_str(), str.size());
	ofs << tmp << '\n';
	ofs.close();
#endif
}


IvrLog::~IvrLog(void)
{
	if (0 != _instance.get()) {
		_instance.release();
	}
}
