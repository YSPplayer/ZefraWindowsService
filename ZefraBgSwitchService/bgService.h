#ifndef _BGSERVICE_
#define _BGSERVICE_
#pragma once
#include <windows.h>
#include <wtsapi32.h>
#include <userenv.h>
#include "../ZefraServiceToos/ZefraServiceToos.h"
#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "Userenv.lib")
namespace zefraBgService {
	#define SLEEP_TIME 3000 //线程的休眠时间
	#define IPSERVICENAME  L"BgService"
	#define SUCCESS 1
	#define ERROR 0
	class BgService{
	public:
		std::string running_path;//运行路径名
		std::string log_path;//日志路径
		std::string pics_path;//图片路径
		BgService(){
			running_path="";
			log_path="";
			pics_path="";
		}
		~BgService(){}
	};
	inline int WriteToLog(const char* str);
	BgService* getBgService();
	VOID ServiceMain(DWORD dwNumServicesArgs, LPWSTR *lpServiceArgVectors);
	VOID WINAPI Ctrlhandler(DWORD request);//控制处理器
	int InitService();
}

#endif 