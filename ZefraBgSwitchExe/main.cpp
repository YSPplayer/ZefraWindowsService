#include <windows.h>
#include <stdio.h>
#include <ctime>
#include <vector> 
#include <string>
#include <fstream>
#include <sstream>
#include "include/ZefraServiceToos.h"
#include "include/nlohmann/json.hpp"//这里使用json来配置我们的文件
using json = nlohmann::json;
char* log_path = NULL;
char* running_path = NULL;
int SwitchingTime = 600000;//这个我们给默认值
std::string bg_Path = "";
bool openService = true;
//服务的名称
#define IPSERVICENAME  L"BgService"
//10分钟换一次壁纸 600000
#define SLEEP_TIME SwitchingTime
#define SERVICEFLAG openService
int WriteToLog(const char * str)
{
    return zServiceToos::WriteToLog(str,log_path);
}
//释放我们的句柄
void CloseHandle(SC_HANDLE h1,SC_HANDLE h2)
{
    if(h1) CloseServiceHandle(h1);
    if(h1) CloseServiceHandle(h2);
}
//检查我们的主服务是否已经被停用
bool CheckServiceState()
{
    SC_HANDLE hManager = NULL;
    SC_HANDLE hService = NULL;
    SERVICE_STATUS ssStatus;
    // 打开 SCM 数据库
    hManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hManager == NULL)
    {
        WriteToLog("【exe】OpenSCManager failed!");
        return false;
    }
    // 打开服务
    hService = OpenService(hManager, IPSERVICENAME, SERVICE_QUERY_STATUS);
    if (hService == NULL)
    {
        WriteToLog("【exe】OpenService failed!");
        CloseHandle(hManager,hService);
        return false;
    }
    // 查询服务状态
    if (!QueryServiceStatus(hService, &ssStatus))
    {
        WriteToLog("【exe】CheckServiceState QueryServiceStatus failed!");
        CloseHandle(hManager,hService);
        return false;
    }
    bool flag = true;
    if (ssStatus.dwCurrentState == SERVICE_STOPPED)
    {
        WriteToLog("【exe】The service is stopped!");
        flag = false;
    } 
    CloseHandle(hManager,hService);
    return flag;
}
//检查我们的主服务是否已退出
bool CheckService()
{
    /*
    这里我们要先查询我们的服务中是否有创建我们的主服务，没有或服务处于通用状态，
    我们就关闭掉该进程
    */
    SC_HANDLE scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scmHandle == NULL) {
        // 打开SCM数据库失败,输出错误信息，停用进程
        WriteToLog("【exe】Failed to open SCM database!");
        return false;
    }
    SC_HANDLE serviceHandle = OpenService(scmHandle, IPSERVICENAME, SERVICE_QUERY_CONFIG);
    if (serviceHandle == NULL) {
        // 打开指定名称的服务失败，输出错误信息
        WriteToLog("【exe】Failed to open service!");
        CloseServiceHandle(scmHandle); //关闭SCM数据库句柄
        return false;
    }
    QUERY_SERVICE_CONFIG serviceConfig;
    DWORD dwBytesNeeded = 0;
    if (!QueryServiceConfig(serviceHandle, &serviceConfig, sizeof(serviceConfig), &dwBytesNeeded)) {
        // 查询服务配置信息失败，输出错误信息
        std::string str = std::to_string(GetLastError());
        char* res  = NULL;
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            res = zServiceToos::CompanChar("【exe】QueryServiceConfig failed with error code:",str.data());
            WriteToLog(res);
            CloseHandle(serviceHandle,scmHandle);
            return false;
        } else {
            //表示缓存区溢出的错误
           WriteToLog("【exe】Failed query service becaseof cache overflow,will retry");
            LPQUERY_SERVICE_CONFIG lpqscBuffer = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LPTR, dwBytesNeeded);
            if (lpqscBuffer == nullptr) {
                WriteToLog("【exe】LocalAlloc failed");
                CloseHandle(serviceHandle,scmHandle);
                return false;
            }
            if (!QueryServiceConfig(serviceHandle, lpqscBuffer, dwBytesNeeded, &dwBytesNeeded)) {
                WriteToLog("【exe】Failed query with overflow retry");
                LocalFree(lpqscBuffer);
                CloseHandle(serviceHandle,scmHandle);
                return false;
            }
            WriteToLog("【exe】success query with overflow retry");
            if (lpqscBuffer && (lpqscBuffer->dwStartType == SERVICE_DISABLED 
                || lpqscBuffer->dwStartType == SERVICE_SYSTEM_START)) {
                WriteToLog("【exe】Service is disabled.");
                LocalFree(lpqscBuffer);
                CloseHandle(serviceHandle,scmHandle);
                return false;
            } else {
                LocalFree(lpqscBuffer);
                CloseHandle(serviceHandle,scmHandle);
            }

        }
    } else {

        // 判断服务是否处于启用状态
        if (serviceConfig.dwStartType == SERVICE_DISABLED || serviceConfig.dwStartType == SERVICE_SYSTEM_START) {
            WriteToLog("【exe】Service is disabled.");
            CloseHandle(serviceHandle,scmHandle);
            return false;
        }
        CloseHandle(serviceHandle,scmHandle);
    }
    return true;
}
//把我们的服务写入注册表中，并让它可以在开机时自动启动
void OpenAutoService() 
{
    // 添加服务到注册表中
    //注意，这里只有写入用户中的run项目HKEY_CURRENT_USER才有效，非用户写入无效
    HKEY hKey = NULL;
    LONG lRes = RegCreateKey(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), &hKey);
    if (lRes != ERROR_SUCCESS)
    {
        WriteToLog("【exe】RegCreateKey failed to open regedit");//写入的路径
        return;
    }
    //szPath是我们要注册到系统表中的exe程序名
    TCHAR szValue[MAX_PATH];
    char * exepath = zServiceToos::CompanChar(running_path,"\\ZefraBgSwitchService.exe");
    //写入以下命令来开机自启动我们的服务，通过VBS脚本来运行
    //wscript.exe "C:\Path\To\Your\VBS\File.vbs"
    //cmd.exe /c sc create BgService binpath= %CD%\ZefraBgSwitchService.exe & sc config BgService start= AUTO & net start BgService
    wsprintf(szValue, TEXT("cmd.exe /c sc create BgService binpath= \"%s\" & sc config BgService start= AUTO & net start BgService"),zServiceToos::CharToWchar(exepath));
    zServiceToos::DestoryPoint(exepath);
    //写入注册表
    if (RegSetValueEx(hKey, IPSERVICENAME, 0, REG_SZ, (BYTE*)szValue, lstrlen(szValue) * sizeof(TCHAR)) != ERROR_SUCCESS)
    {
        WriteToLog("【exe】RegSetValueEx failed to change regedit");//写入的路径
        return;
    }
    //关闭句柄
    RegCloseKey(hKey);
    WriteToLog("【exe】successly to change regedit for open service!");
    return;
}
void CloseAutoService() 
{
    //删除注册表里的启动项
    HKEY hKey = NULL;
    LONG lRes = RegOpenKey(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), &hKey);
    if (lRes == ERROR_SUCCESS)
    {
        lRes = RegDeleteValue(hKey, IPSERVICENAME);
        RegCloseKey(hKey);
        WriteToLog("【exe】successly to change regedit for close service!");
        return;
    }
    WriteToLog("【exe】RegDeleteValue failed to change regedit");//删除的路径
    return;
}
// 打开指定路径的 JSON 文件并解析其中的数据
void ParseJsonFile(const std::string& path) {
    // 打开文件
    std::ifstream file(path);
    if (!file.is_open()) {
        WriteToLog("【exe】parse_json_file Failed to open file ");
        return;
    }
    // 读取文件内容
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::wstring content = zServiceToos::CharToWchar(buffer.str().data());
    //解析JSON数据
    try {
        json j = json::parse(content);
        SwitchingTime = j["Service"]["SwitchingTime"];
        bg_Path = j["Service"]["bgPath"];
        openService = j["Service"]["openService"];
        WriteToLog("【exe】load json successly!");
    } catch (const std::exception& e) {
        WriteToLog("【exe】faild to parse json file!");
        return;
    }
}
int main()
{
    running_path = zServiceToos::GetRootPath();//获取文件的启动目录
    log_path = zServiceToos::CompanChar(running_path,"\\log\\run.log");
    char* json_path = zServiceToos::CompanChar(running_path,"\\config.json");
    ParseJsonFile(json_path);//加载我们的josn文件
    std::string pics_path = bg_Path != "" ? bg_Path.data() : zServiceToos::CompanChar(running_path,"\\pics\\");
    char *before_path = zServiceToos::CompanChar(pics_path.data(),"*");
    const wchar_t* folder_path = zServiceToos::CharToWchar(before_path);//我们查找文件的路径
    WIN32_FIND_DATAW file_data;//定义 WIN32_FIND_DATAW 结构体，用于保存查找到的文件信息
    HANDLE file_handle = FindFirstFileW(folder_path, &file_data);//查找第一个匹配的文件
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        //这里是匹配不到文件夹
        WriteToLog("【exe】WIN32_FIND_DATAW Failed to open folder!");
        return 0;
    }
    //定义一个vector容器来存储我们的数据
    std::vector<std::wstring> *V = new std::vector<std::wstring>();
    do
    {
        // 如果是文件我们才需要
        if (!(file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            std::wstring fileName = file_data.cFileName;
            std::wstring fileExt = fileName.substr(fileName.rfind(L".") + 1);
            if (fileExt == L"png" || fileExt == L"jpg") {
                //存储我们的值
                V->push_back(file_data.cFileName);
            }
        }
    } while (FindNextFileW(file_handle, &file_data));
    //关闭查找
    FindClose(file_handle);
    //释放指针的内存
    zServiceToos::DestoryPoint(before_path);
    //这里把我们的服务注册到注册表里，这样我们就可以开始自动启动了
    SERVICEFLAG ? OpenAutoService() : CloseAutoService();
    srand(time(0));//设置随机数种子
    while (true)
    {
        if(SLEEP_TIME < 0) SLEEP_TIME = 0;
        Sleep(SLEEP_TIME);
        //如果我们服务关闭的话就退出进程
        if(!CheckService() || !CheckServiceState()) break;
        int index = zServiceToos::GetRandomNumber(0,V->size());
        if(index >= V->size() || index < 0) index = 0;
        char* resPath = zServiceToos::CompanChar(pics_path.data(),zServiceToos::WcharToChar(V->at(index).data()));
        char *log = zServiceToos::CompanChar("【exe】load pics from path:",resPath);
        WriteToLog(log);//写入的路径
        std::wstring wallpaper_path = zServiceToos::CharToWchar(resPath);
        //添加壁纸
        if(!SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, zServiceToos::WcharToChar(wallpaper_path.data()), SPIF_UPDATEINIFILE)) 
        {
           WriteToLog("【exe】SystemParametersInfoA error to change pics");
            return 0;
        } 
        WriteToLog("【exe】SystemParametersInfoA successly to change pics");
        zServiceToos::DestoryPoint(resPath);
        zServiceToos::DestoryPoint(log);
    }
    zServiceToos::DestoryPoint(running_path);
    zServiceToos::DestoryPoint(V,false);
    WriteToLog("【exe】close exe successfuly");
    /*
    * 下面使用修改注册表的方式修改
            //添加壁纸
        SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, WcharToChar(wallpaper_path.data()), SPIF_UPDATEINIFILE);
        return 1;
     HKEY hKey;
    DWORD dwDisposition;
    if(RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Control Panel\\Desktop"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS)
    {
        // 打开成功
        WriteToLog("Successfully opened key!");
        std::cout << "Successfully opened key!" << std::endl;
        LPCTSTR lpValueName = TEXT("Wallpaper");
        LPCTSTR lpData = wallpaper_path.data();
        WriteToLog(WcharToChar(wallpaper_path.data()));
        std::cout << WcharToChar(wallpaper_path.data())<< std::endl;
        DWORD cbData = sizeof(TCHAR) * (_tcslen(lpData) + 1);
        if(RegSetValueEx(hKey, lpValueName, 0, REG_SZ, (BYTE*)lpData, cbData) == ERROR_SUCCESS)
        {
            // 修改成功
            WriteToLog("Successfully change key!");
            std::cout << "Successfully change key!" << std::endl;
            if(SystemParametersInfo(SPI_SETDESKWALLPAPER, 0,NULL,SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE)) {
                std::cout << "SystemParametersInfo Successfully to change key!" << std::endl;
                WriteToLog("SystemParametersInfo Successfully to change key!");
            } else {
                std::cout << "SystemParametersInfo Failed to change key!" << std::endl;
                WriteToLog("SystemParametersInfo Failed to change key!");
            }
        } else {
            std::cout << "Failed to change key!" << std::endl;
            WriteToLog("Failed to change key!");
        }
        RegCloseKey(hKey);
    }else {
        std::cout << "Failed to open key!" << std::endl;
        WriteToLog("Failed to open key!");
    }
    */
    return 1;
}