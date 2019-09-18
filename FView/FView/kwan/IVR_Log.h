/*
简单的日志记录类. (日志而已，何必那么复杂！！！）
W.J.Chang 2013.12.13

说明:(IvrLog.h)
1, 简单的单件实现（自动垃圾回收）
2, 使用方法：IvrLog::Inst()->Log("Run...");
3, 日志记录结果：Run...    [2013.12.13 16:38:42 Friday]
*/
#pragma once  
#ifndef EASY_LOG_H_8080  
#define EASY_LOG_H_8080  
#include <memory>  
#include <ctime>  
#include <iostream>  
#include <fstream>  
#include <tchar.h>

// 日志格式[时间][模块ID][日志级别][日志内容]
#define LOG_FORMAT "[%s][%d][%d][%s]\r\n"
// 日志缓冲区大小
#define LOG_BUFFER_SIZE 1024

class IvrLog
{
public:
	static IvrLog * Inst() {
		if (0 == _instance.get()) {
			_instance.reset(new IvrLog);
		}
		return _instance.get();
	}
	void Log(std::string msg); // 写日志的方法  
	void Log(TCHAR* msg); // 写日志的方法  
	void Log(std::wstring loginfo);
private:
	IvrLog(void) {}
	virtual ~IvrLog(void);
	friend class std::auto_ptr<IvrLog>;
	static std::auto_ptr<IvrLog> _instance;

	const char* filename = "FAR_debug.log";
};


//IvrLog::Inst()->Log("Run...");
#endif  
