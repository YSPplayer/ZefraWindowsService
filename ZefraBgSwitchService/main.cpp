#include "bgService.h"

using namespace zefraBgService;
using namespace zServiceToos;
/*
创建指令 :sc create 服务名 binpath=
删除指令:sc delete 服务名
查询服务指令：services.msc
*/
int main() {
    WriteToLog("【Service】Servicemain start");
    char * rootPath = GetRootPath();
    getBgService()->running_path = rootPath;//保存我们的路径
    DestoryPoint(rootPath);
    SERVICE_TABLE_ENTRY ServiceTable[2];
    ServiceTable[0].lpServiceName = new wchar_t[]{IPSERVICENAME};//wchar * 这里用const wchar*报错
    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;//调用我们的函数
    ServiceTable[1].lpServiceName = NULL;
    ServiceTable[1].lpServiceProc = NULL;
    //启动服务的控制分派机线程
    StartServiceCtrlDispatcher(ServiceTable);
    return 0;
}