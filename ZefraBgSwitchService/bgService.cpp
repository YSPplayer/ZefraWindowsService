#include "bgService.h"
bool brun = false;
SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE  hStatus;
zefraBgService::BgService bgService;
zefraBgService::BgService* zefraBgService::getBgService()//��ȡ���ǵ���
{
    return &bgService;
}
inline int zefraBgService::WriteToLog(const char* str) {
    return zServiceToos::WriteToLog(str,bgService.log_path.data());
}
int zefraBgService::InitService() {
    return WriteToLog("��Service��Monitoring started.");
}
VOID WINAPI zefraBgService::Ctrlhandler(DWORD request) {
    switch (request)
    {
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
            brun = false;
            serviceStatus.dwCurrentState = SERVICE_STOPPED;
            break;
        case SERVICE_CONTROL_PAUSE:
            brun = false;
            serviceStatus.dwCurrentState = SERVICE_PAUSED;
            break;
        case SERVICE_CONTROL_CONTINUE:
            brun = false;
            serviceStatus.dwCurrentState = SERVICE_RUNNING;
            break;
        default:
            break;
    }
    //��SCM���桰SERVICE_STOPPED��״̬
    SetServiceStatus(hStatus,&serviceStatus);
}
VOID zefraBgService::ServiceMain(DWORD dwNumServicesArgs, LPWSTR *lpServiceArgVectors) {//������������е�������   
    WriteToLog("��Service��ServiceMain start");
    char *log_path = zServiceToos::CompanChar(bgService.running_path.data(),"\\log\\run.log");
    char *pics_path = zServiceToos::CompanChar(bgService.running_path.data(),"\\log\\run.log");
    bgService.log_path = log_path;//string�ǿ���һ���������
    bgService.pics_path = pics_path;
    zServiceToos::DestoryPoint(log_path);
    zServiceToos::DestoryPoint(pics_path);
    serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;//����Ϊ����ԱȨ��ģʽ
   // serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    //����ǰ״̬���������ʱ���ʼ��δ��ɣ�����pending���������˼
    serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    //֪ͨSCM��������ĸ��򣬴������������洦��
    serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;  //����ֻ����ϵͳ�ػ���ֹͣ����2�ֿ�������
    //��ֹ���񲢱����˳�ϸ�ڣ���ʼ��ʱ���˳��������Ǹ�ֵΪ0
    serviceStatus.dwWin32ExitCode =  0;
    serviceStatus.dwServiceSpecificExitCode = 0; //��ʾ��ʼ��ĳ���������Ҫ30s���ϣ���Ϊ���ǵ�ʵ����ʼ�����̺̣ܶ�����Ϊ0
    serviceStatus.dwCheckPoint = 0;
    serviceStatus.dwWaitHint = 0;
    //Ϊ����ע����ƴ�����
    hStatus = RegisterServiceCtrlHandler(IPSERVICENAME,Ctrlhandler);//��������ָ��controlhandlefunctionָ��
    if (!hStatus)
    {
        //��¼ʧ��
        WriteToLog("��Service��Register ServerCtrlhandler Failed");
        return;
    }
    //��¼�ɹ�
    WriteToLog("��Service��Register ServiceCtrlHandler Sucess");
    if(InitService() == ERROR)
    {
        //��ʼ��ʧ�ܣ�����д��־�ļ�ʧ��
        serviceStatus.dwCurrentState = SERVICE_STOPPED;
        serviceStatus.dwWin32ExitCode = -1;
        SetServiceStatus(hStatus,&serviceStatus);
        //�˳�serviceMain
        return;
    }
    //��SCM�������з���״̬
    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    BOOL bstatus = SetServiceStatus(hStatus,&serviceStatus);
    if(!bstatus)//����������ʧ�ܣ���¼����ʧ�ܵĴ���
    {
        DWORD derror = GetLastError();
        char str[100]={0};
        sprintf_s(str,100,"��Service��SetServiceStatus Failed,The error value is %d",derror);
        WriteToLog(str);
        return;
    }
    brun = true;
    while (brun)
    {
        /*
          ���ﴴ��exe�̣߳�ͨ��exe��ִ���л���ֽ���߼�����Ϊ���ǵ�HKEY_CURRENT_USER�µ�ע��
          ����ͨ�������޸ģ���Ϊ�����û��ҹ���ֻ����������exe�޸�
        */
        //���������õ�һ������ѭ������
        char * excharPath = zServiceToos::CompanChar(bgService.running_path.data(),"\\ZefraBgSwitchExe.exe");
        std::string exe_path = excharPath;
        zServiceToos::DestoryPoint(excharPath);
        DWORD dwSessionId = WTSGetActiveConsoleSessionId(); // ��ȡ��ǰ�ỰID
        HANDLE hToken = NULL;

        if (!WTSQueryUserToken(dwSessionId, &hToken)) { // ��ȡ�Ự��������
            WriteToLog("��Service��Failed to get user token");
            break;
        }
        PROCESS_INFORMATION pi;
        STARTUPINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        const wchar_t* szExample = _T("winsta0\\default");
        size_t len = wcslen(szExample) + 1;
        LPWSTR lpwstr = new WCHAR[len];
        ZeroMemory(lpwstr, len * sizeof(WCHAR));
        wcscpy_s(lpwstr, len, szExample);
        //CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT
        si.lpDesktop = lpwstr; // ʹ��Ĭ������
        si.dwFlags = STARTF_USESHOWWINDOW;  // ʹ��ָ���Ĵ���״̬
        si.wShowWindow = SW_HIDE;           // ���ش���
        BOOL bRet = CreateProcessAsUser(hToken, // ʹ�÷������ƴ����½���
            NULL,
            zServiceToos::CharToWchar(exe_path.data()),
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT,
            NULL,
            NULL,
            &si,
            &pi);
        if (!bRet) {
            WriteToLog("��Service��Failed to create process as user");
        } else {
            WriteToLog("��Service��success to create process as user");
            WaitForSingleObject(pi.hProcess, INFINITE); // �ȴ������˳�
            CloseHandle(pi.hProcess);//�ͷ���Դ
            CloseHandle(pi.hThread);//�ͷ���Դ
            CloseHandle(hToken);//�ͷ���Դ
        }
       // Sleep(SLEEP_TIME);//�߳�����ʱ��
        break;
    }
    WriteToLog("��Service��Service Stopped");
    return;
}