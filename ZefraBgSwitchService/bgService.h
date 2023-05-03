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
	#define SLEEP_TIME 3000 //�̵߳�����ʱ��
	#define IPSERVICENAME  L"BgService"
	#define SUCCESS 1
	#define ERROR 0
	class BgService{
	public:
		std::string running_path;//����·����
		std::string log_path;//��־·��
		std::string pics_path;//ͼƬ·��
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
	VOID WINAPI Ctrlhandler(DWORD request);//���ƴ�����
	int InitService();
}

#endif 