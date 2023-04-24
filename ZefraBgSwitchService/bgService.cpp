#include "bgService.h"
bool brun = false;
SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE  hStatus;
zefraBgService::BgService bgService;
zefraBgService::BgService* zefraBgService::getBgService()//获取我们的类
{
    return &bgService;
}
inline int zefraBgService::WriteToLog(const char* str) {
    return zServiceToos::WriteToLog(str,bgService.log_path.data());
}
int zefraBgService::InitService() {
    return WriteToLog("【Service】Monitoring started.");
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
    //向SCM报告“SERVICE_STOPPED”状态
    SetServiceStatus(hStatus,&serviceStatus);
}
VOID zefraBgService::ServiceMain(DWORD dwNumServicesArgs, LPWSTR *lpServiceArgVectors) {//这个是我们运行的主函数   
    WriteToLog("【Service】ServiceMain start");
    char *log_path = zServiceToos::CompanChar(bgService.running_path.data(),"\\log\\run.log");
    char *pics_path = zServiceToos::CompanChar(bgService.running_path.data(),"\\log\\run.log");
    bgService.log_path = log_path;//string是拷贝一个对象过来
    bgService.pics_path = pics_path;
    zServiceToos::DestoryPoint(log_path);
    zServiceToos::DestoryPoint(pics_path);
    serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;//设置为管理员权限模式
   // serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    //服务当前状态，在这里的时候初始化未完成，所以pending，挂起的意思
    serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    //通知SCM服务接收哪个域，处理控制请求后面处理
    serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;  //本例只接受系统关机和停止服务2种控制命令
    //终止服务并报告退出细节，初始化时不退出，将他们赋值为0
    serviceStatus.dwWin32ExitCode =  0;
    serviceStatus.dwServiceSpecificExitCode = 0; //表示初始化某个服务进程要30s以上，因为我们的实例初始化过程很短，设置为0
    serviceStatus.dwCheckPoint = 0;
    serviceStatus.dwWaitHint = 0;
    //为服务注册控制处理器
    hStatus = RegisterServiceCtrlHandler(IPSERVICENAME,Ctrlhandler);//服务名，指向controlhandlefunction指针
    if (!hStatus)
    {
        //登录失败
        WriteToLog("【Service】Register ServerCtrlhandler Failed");
        return;
    }
    //登录成功
    WriteToLog("【Service】Register ServiceCtrlHandler Sucess");
    if(InitService() == ERROR)
    {
        //初始化失败，可能写日志文件失败
        serviceStatus.dwCurrentState = SERVICE_STOPPED;
        serviceStatus.dwWin32ExitCode = -1;
        SetServiceStatus(hStatus,&serviceStatus);
        //退出serviceMain
        return;
    }
    //向SCM报告运行服务状态
    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    BOOL bstatus = SetServiceStatus(hStatus,&serviceStatus);
    if(!bstatus)//这里是运行失败，记录运行失败的错误
    {
        DWORD derror = GetLastError();
        char str[100]={0};
        sprintf_s(str,100,"【Service】SetServiceStatus Failed,The error value is %d",derror);
        WriteToLog(str);
        return;
    }
    brun = true;
    while (brun)
    {
        /*
          这里创建exe线程，通过exe来执行切换壁纸的逻辑，因为我们的HKEY_CURRENT_USER下的注册
          表不能通过事务修改，因为它和用户挂钩，只能用正常的exe修改
        */
        //这里我们让第一个服务循环调用
        char * excharPath = zServiceToos::CompanChar(bgService.running_path.data(),"\\ZefraBgSwitchExe.exe");
        std::string exe_path = excharPath;
        zServiceToos::DestoryPoint(excharPath);
        DWORD dwSessionId = WTSGetActiveConsoleSessionId(); // 获取当前会话ID
        HANDLE hToken = NULL;

        if (!WTSQueryUserToken(dwSessionId, &hToken)) { // 获取会话访问令牌
            WriteToLog("【Service】Failed to get user token");
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
        si.lpDesktop = lpwstr; // 使用默认桌面
        si.dwFlags = STARTF_USESHOWWINDOW;  // 使用指定的窗口状态
        si.wShowWindow = SW_HIDE;           // 隐藏窗口
        BOOL bRet = CreateProcessAsUser(hToken, // 使用访问令牌创建新进程
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
            WriteToLog("【Service】Failed to create process as user");
        } else {
            WriteToLog("【Service】success to create process as user");
            WaitForSingleObject(pi.hProcess, INFINITE); // 等待进程退出
            CloseHandle(pi.hProcess);//释放资源
            CloseHandle(pi.hThread);//释放资源
            CloseHandle(hToken);//释放资源
        }
       // Sleep(SLEEP_TIME);//线程休眠时间
        break;
    }
    WriteToLog("【Service】Service Stopped");
    return;
}