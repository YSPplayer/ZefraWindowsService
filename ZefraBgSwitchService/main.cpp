#include "bgService.h"

using namespace zefraBgService;
using namespace zServiceToos;
/*
����ָ�� :sc create ������ binpath=
ɾ��ָ��:sc delete ������
��ѯ����ָ�services.msc
*/
int main() {
    WriteToLog("��Service��Servicemain start");
    char * rootPath = GetRootPath();
    getBgService()->running_path = rootPath;//�������ǵ�·��
    DestoryPoint(rootPath);
    SERVICE_TABLE_ENTRY ServiceTable[2];
    ServiceTable[0].lpServiceName = new wchar_t[]{IPSERVICENAME};//wchar * ������const wchar*����
    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;//�������ǵĺ���
    ServiceTable[1].lpServiceName = NULL;
    ServiceTable[1].lpServiceProc = NULL;
    //��������Ŀ��Ʒ��ɻ��߳�
    StartServiceCtrlDispatcher(ServiceTable);
    return 0;
}