#include <windows.h>
#include <stdio.h>
#include <ctime>
#include <vector> 
#include<string>
#include"include/ZefraServiceToos.h"
#define IPSERVICENAME  L"BgService"
bool CheckService(char* log_path)
{
    /*
    这里我们要先查询我们的服务中是否有创建我们的主服务，没有或服务处于通用状态，
    我们就关闭掉该进程
    */
    SC_HANDLE scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scmHandle == NULL) {
        // 打开SCM数据库失败,输出错误信息，停用进程
        zServiceToos::WriteToLog("【exe】Failed to open SCM database!",log_path);
        return false;
    }
    SC_HANDLE serviceHandle = OpenService(scmHandle, IPSERVICENAME, SERVICE_QUERY_CONFIG);
    if (serviceHandle == NULL) {
        // 打开指定名称的服务失败，输出错误信息
        zServiceToos::WriteToLog("【exe】Failed to open service!",log_path);
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
            zServiceToos::WriteToLog(res,log_path);
            CloseServiceHandle(serviceHandle); //关闭服务句柄
            CloseServiceHandle(scmHandle); // 关闭SCM数据库句柄
            delete[] res;
            res = NULL;
            return false;
        } else {
            //表示缓存区溢出的错误
            zServiceToos::WriteToLog("【exe】Failed query service becaseof cache overflow,will retry",log_path);
            LPQUERY_SERVICE_CONFIG lpqscBuffer = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LPTR, dwBytesNeeded);
            if (lpqscBuffer == nullptr) {
                zServiceToos::WriteToLog("【exe】LocalAlloc failed",log_path);
                CloseServiceHandle(serviceHandle); //关闭服务句柄
                CloseServiceHandle(scmHandle); // 关闭SCM数据库句柄
                return false;
            }
            if (!QueryServiceConfig(serviceHandle, lpqscBuffer, dwBytesNeeded, &dwBytesNeeded)) {
                zServiceToos::WriteToLog("【exe】Failed query with overflow retry",log_path);
                LocalFree(lpqscBuffer);
                CloseServiceHandle(serviceHandle);
                CloseServiceHandle(scmHandle); 
                return false;
            }
            zServiceToos::WriteToLog("【exe】success query with overflow retry",log_path);
            if (lpqscBuffer && (lpqscBuffer->dwStartType == SERVICE_DISABLED 
                || lpqscBuffer->dwStartType == SERVICE_SYSTEM_START)) {
                zServiceToos::WriteToLog("【exe】Service is disabled.",log_path);
                LocalFree(lpqscBuffer);
                CloseServiceHandle(serviceHandle);
                CloseServiceHandle(scmHandle);
                return false;
            } else {
                LocalFree(lpqscBuffer);
                CloseServiceHandle(serviceHandle);
                CloseServiceHandle(scmHandle);
            }

        }
    } else {

        // 判断服务是否处于启用状态
        if (serviceConfig.dwStartType == SERVICE_DISABLED || serviceConfig.dwStartType == SERVICE_SYSTEM_START) {
            zServiceToos::WriteToLog("【exe】Service is disabled.",log_path);
            CloseServiceHandle(serviceHandle);
            CloseServiceHandle(scmHandle);
            return false;
        }
        CloseServiceHandle(serviceHandle);
        CloseServiceHandle(scmHandle);
    }
}
int main()
{
    //10分钟换一次壁纸
    #define SLEEP_TIME 600000
    char* running_path = zServiceToos::GetRootPath();//获取文件的启动目录
    char* log_path = zServiceToos::CompanChar(running_path,"\\log\\run.log");
    char* pics_path = zServiceToos::CompanChar(running_path,"\\pics\\");
    char *before_path = zServiceToos::CompanChar(running_path,"\\pics\\*");
    const wchar_t* folder_path = zServiceToos::CharToWchar(before_path);//我们查找文件的路径
    WIN32_FIND_DATAW file_data;//定义 WIN32_FIND_DATAW 结构体，用于保存查找到的文件信息
    delete[] before_path;//释放内存
    before_path = NULL;
    HANDLE file_handle = FindFirstFileW(folder_path, &file_data);//查找第一个匹配的文件
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        //这里是匹配不到文件夹
        zServiceToos::WriteToLog("【exe】WIN32_FIND_DATAW Failed to open folder!",log_path);
        return 0;
    }
    //定义一个vector容器来存储我们的数据
    std::vector<std::wstring> V;
    do
    {
        // 如果是文件我们才需要
        if (!(file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            std::wstring fileName = file_data.cFileName;
            std::wstring fileExt = fileName.substr(fileName.rfind(L".") + 1);
            if (fileExt == L"png" || fileExt == L"jpg") {
                //存储我们的值
                V.push_back(file_data.cFileName);
            }
        }
    } while (FindNextFileW(file_handle, &file_data));
    //关闭查找
    FindClose(file_handle);
    srand(time(0));//设置随机数种子
    while (true)
    {
        //如果我们服务关闭的话就退出进程
        if(!CheckService(log_path)) break;
        UINT32 index = zServiceToos::GetRandomNumber(0,V.size());
        if(index >= V.size() || index < 0) index = 0;
        char* resPath = zServiceToos::CompanChar(pics_path,zServiceToos::WcharToChar(V.at(index).data()));
        char *log = zServiceToos::CompanChar("【exe】load pics from path:",resPath);
        zServiceToos::WriteToLog(log,log_path);//写入的路径
        std::wstring wallpaper_path = zServiceToos::CharToWchar(resPath);
        //添加壁纸
        if(!SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, zServiceToos::WcharToChar(wallpaper_path.data()), SPIF_UPDATEINIFILE)) 
        {
            zServiceToos::WriteToLog("【exe】SystemParametersInfoA error to change pics",log_path);
            return 0;
        } 
        zServiceToos::WriteToLog("【exe】SystemParametersInfoA successly to change pics",log_path);
        delete[] resPath;//释放内存
        resPath = NULL;
        delete[] log;
        log = NULL;
        Sleep(SLEEP_TIME);
    }
    
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