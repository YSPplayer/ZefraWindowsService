#include <windows.h>
#include <stdio.h>
#include <ctime>
#include <vector> 
#include <string>
#include <fstream>
#include <sstream>
#include "include/ZefraServiceToos.h"
#include "include/nlohmann/json.hpp"//����ʹ��json���������ǵ��ļ�
using json = nlohmann::json;
char* log_path = NULL;
char* running_path = NULL;
int SwitchingTime = 600000;//������Ǹ�Ĭ��ֵ
std::string bg_Path = "";
bool openService = true;
//���������
#define IPSERVICENAME  L"BgService"
//10���ӻ�һ�α�ֽ 600000
#define SLEEP_TIME SwitchingTime
#define SERVICEFLAG openService
int WriteToLog(const char * str)
{
    return zServiceToos::WriteToLog(str,log_path);
}
//�ͷ����ǵľ��
void CloseHandle(SC_HANDLE h1,SC_HANDLE h2)
{
    if(h1) CloseServiceHandle(h1);
    if(h1) CloseServiceHandle(h2);
}
//������ǵ��������Ƿ��Ѿ���ͣ��
bool CheckServiceState()
{
    SC_HANDLE hManager = NULL;
    SC_HANDLE hService = NULL;
    SERVICE_STATUS ssStatus;
    // �� SCM ���ݿ�
    hManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hManager == NULL)
    {
        WriteToLog("��exe��OpenSCManager failed!");
        return false;
    }
    // �򿪷���
    hService = OpenService(hManager, IPSERVICENAME, SERVICE_QUERY_STATUS);
    if (hService == NULL)
    {
        WriteToLog("��exe��OpenService failed!");
        CloseHandle(hManager,hService);
        return false;
    }
    // ��ѯ����״̬
    if (!QueryServiceStatus(hService, &ssStatus))
    {
        WriteToLog("��exe��CheckServiceState QueryServiceStatus failed!");
        CloseHandle(hManager,hService);
        return false;
    }
    bool flag = true;
    if (ssStatus.dwCurrentState == SERVICE_STOPPED)
    {
        WriteToLog("��exe��The service is stopped!");
        flag = false;
    } 
    CloseHandle(hManager,hService);
    return flag;
}
//������ǵ��������Ƿ����˳�
bool CheckService()
{
    /*
    ��������Ҫ�Ȳ�ѯ���ǵķ������Ƿ��д������ǵ�������û�л������ͨ��״̬��
    ���Ǿ͹رյ��ý���
    */
    SC_HANDLE scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scmHandle == NULL) {
        // ��SCM���ݿ�ʧ��,���������Ϣ��ͣ�ý���
        WriteToLog("��exe��Failed to open SCM database!");
        return false;
    }
    SC_HANDLE serviceHandle = OpenService(scmHandle, IPSERVICENAME, SERVICE_QUERY_CONFIG);
    if (serviceHandle == NULL) {
        // ��ָ�����Ƶķ���ʧ�ܣ����������Ϣ
        WriteToLog("��exe��Failed to open service!");
        CloseServiceHandle(scmHandle); //�ر�SCM���ݿ���
        return false;
    }
    QUERY_SERVICE_CONFIG serviceConfig;
    DWORD dwBytesNeeded = 0;
    if (!QueryServiceConfig(serviceHandle, &serviceConfig, sizeof(serviceConfig), &dwBytesNeeded)) {
        // ��ѯ����������Ϣʧ�ܣ����������Ϣ
        std::string str = std::to_string(GetLastError());
        char* res  = NULL;
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            res = zServiceToos::CompanChar("��exe��QueryServiceConfig failed with error code:",str.data());
            WriteToLog(res);
            CloseHandle(serviceHandle,scmHandle);
            return false;
        } else {
            //��ʾ����������Ĵ���
           WriteToLog("��exe��Failed query service becaseof cache overflow,will retry");
            LPQUERY_SERVICE_CONFIG lpqscBuffer = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LPTR, dwBytesNeeded);
            if (lpqscBuffer == nullptr) {
                WriteToLog("��exe��LocalAlloc failed");
                CloseHandle(serviceHandle,scmHandle);
                return false;
            }
            if (!QueryServiceConfig(serviceHandle, lpqscBuffer, dwBytesNeeded, &dwBytesNeeded)) {
                WriteToLog("��exe��Failed query with overflow retry");
                LocalFree(lpqscBuffer);
                CloseHandle(serviceHandle,scmHandle);
                return false;
            }
            WriteToLog("��exe��success query with overflow retry");
            if (lpqscBuffer && (lpqscBuffer->dwStartType == SERVICE_DISABLED 
                || lpqscBuffer->dwStartType == SERVICE_SYSTEM_START)) {
                WriteToLog("��exe��Service is disabled.");
                LocalFree(lpqscBuffer);
                CloseHandle(serviceHandle,scmHandle);
                return false;
            } else {
                LocalFree(lpqscBuffer);
                CloseHandle(serviceHandle,scmHandle);
            }

        }
    } else {

        // �жϷ����Ƿ�������״̬
        if (serviceConfig.dwStartType == SERVICE_DISABLED || serviceConfig.dwStartType == SERVICE_SYSTEM_START) {
            WriteToLog("��exe��Service is disabled.");
            CloseHandle(serviceHandle,scmHandle);
            return false;
        }
        CloseHandle(serviceHandle,scmHandle);
    }
    return true;
}
//�����ǵķ���д��ע����У������������ڿ���ʱ�Զ�����
void OpenAutoService() 
{
    // ��ӷ���ע�����
    //ע�⣬����ֻ��д���û��е�run��ĿHKEY_CURRENT_USER����Ч�����û�д����Ч
    HKEY hKey = NULL;
    LONG lRes = RegCreateKey(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), &hKey);
    if (lRes != ERROR_SUCCESS)
    {
        WriteToLog("��exe��RegCreateKey failed to open regedit");//д���·��
        return;
    }
    //szPath������Ҫע�ᵽϵͳ���е�exe������
    TCHAR szValue[MAX_PATH];
    char * exepath = zServiceToos::CompanChar(running_path,"\\ZefraBgSwitchService.exe");
    //д�������������������������ǵķ���ͨ��VBS�ű�������
    //wscript.exe "C:\Path\To\Your\VBS\File.vbs"
    //cmd.exe /c sc create BgService binpath= %CD%\ZefraBgSwitchService.exe & sc config BgService start= AUTO & net start BgService
    wsprintf(szValue, TEXT("cmd.exe /c sc create BgService binpath= \"%s\" & sc config BgService start= AUTO & net start BgService"),zServiceToos::CharToWchar(exepath));
    zServiceToos::DestoryPoint(exepath);
    //д��ע���
    if (RegSetValueEx(hKey, IPSERVICENAME, 0, REG_SZ, (BYTE*)szValue, lstrlen(szValue) * sizeof(TCHAR)) != ERROR_SUCCESS)
    {
        WriteToLog("��exe��RegSetValueEx failed to change regedit");//д���·��
        return;
    }
    //�رվ��
    RegCloseKey(hKey);
    WriteToLog("��exe��successly to change regedit for open service!");
    return;
}
void CloseAutoService() 
{
    //ɾ��ע������������
    HKEY hKey = NULL;
    LONG lRes = RegOpenKey(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), &hKey);
    if (lRes == ERROR_SUCCESS)
    {
        lRes = RegDeleteValue(hKey, IPSERVICENAME);
        RegCloseKey(hKey);
        WriteToLog("��exe��successly to change regedit for close service!");
        return;
    }
    WriteToLog("��exe��RegDeleteValue failed to change regedit");//ɾ����·��
    return;
}
// ��ָ��·���� JSON �ļ����������е�����
void ParseJsonFile(const std::string& path) {
    // ���ļ�
    std::ifstream file(path);
    if (!file.is_open()) {
        WriteToLog("��exe��parse_json_file Failed to open file ");
        return;
    }
    // ��ȡ�ļ�����
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::wstring content = zServiceToos::CharToWchar(buffer.str().data());
    //����JSON����
    try {
        json j = json::parse(content);
        SwitchingTime = j["Service"]["SwitchingTime"];
        bg_Path = j["Service"]["bgPath"];
        openService = j["Service"]["openService"];
        WriteToLog("��exe��load json successly!");
    } catch (const std::exception& e) {
        WriteToLog("��exe��faild to parse json file!");
        return;
    }
}
int main()
{
    running_path = zServiceToos::GetRootPath();//��ȡ�ļ�������Ŀ¼
    log_path = zServiceToos::CompanChar(running_path,"\\log\\run.log");
    char* json_path = zServiceToos::CompanChar(running_path,"\\config.json");
    ParseJsonFile(json_path);//�������ǵ�josn�ļ�
    std::string pics_path = bg_Path != "" ? bg_Path.data() : zServiceToos::CompanChar(running_path,"\\pics\\");
    char *before_path = zServiceToos::CompanChar(pics_path.data(),"*");
    const wchar_t* folder_path = zServiceToos::CharToWchar(before_path);//���ǲ����ļ���·��
    WIN32_FIND_DATAW file_data;//���� WIN32_FIND_DATAW �ṹ�壬���ڱ�����ҵ����ļ���Ϣ
    HANDLE file_handle = FindFirstFileW(folder_path, &file_data);//���ҵ�һ��ƥ����ļ�
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        //������ƥ�䲻���ļ���
        WriteToLog("��exe��WIN32_FIND_DATAW Failed to open folder!");
        return 0;
    }
    //����һ��vector�������洢���ǵ�����
    std::vector<std::wstring> *V = new std::vector<std::wstring>();
    do
    {
        // ������ļ����ǲ���Ҫ
        if (!(file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            std::wstring fileName = file_data.cFileName;
            std::wstring fileExt = fileName.substr(fileName.rfind(L".") + 1);
            if (fileExt == L"png" || fileExt == L"jpg") {
                //�洢���ǵ�ֵ
                V->push_back(file_data.cFileName);
            }
        }
    } while (FindNextFileW(file_handle, &file_data));
    //�رղ���
    FindClose(file_handle);
    //�ͷ�ָ����ڴ�
    zServiceToos::DestoryPoint(before_path);
    //��������ǵķ���ע�ᵽע�����������ǾͿ��Կ�ʼ�Զ�������
    SERVICEFLAG ? OpenAutoService() : CloseAutoService();
    srand(time(0));//�������������
    while (true)
    {
        if(SLEEP_TIME < 0) SLEEP_TIME = 0;
        Sleep(SLEEP_TIME);
        //������Ƿ���رյĻ����˳�����
        if(!CheckService() || !CheckServiceState()) break;
        int index = zServiceToos::GetRandomNumber(0,V->size());
        if(index >= V->size() || index < 0) index = 0;
        char* resPath = zServiceToos::CompanChar(pics_path.data(),zServiceToos::WcharToChar(V->at(index).data()));
        char *log = zServiceToos::CompanChar("��exe��load pics from path:",resPath);
        WriteToLog(log);//д���·��
        std::wstring wallpaper_path = zServiceToos::CharToWchar(resPath);
        //��ӱ�ֽ
        if(!SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, zServiceToos::WcharToChar(wallpaper_path.data()), SPIF_UPDATEINIFILE)) 
        {
           WriteToLog("��exe��SystemParametersInfoA error to change pics");
            return 0;
        } 
        WriteToLog("��exe��SystemParametersInfoA successly to change pics");
        zServiceToos::DestoryPoint(resPath);
        zServiceToos::DestoryPoint(log);
    }
    zServiceToos::DestoryPoint(running_path);
    zServiceToos::DestoryPoint(V,false);
    WriteToLog("��exe��close exe successfuly");
    /*
    * ����ʹ���޸�ע���ķ�ʽ�޸�
            //��ӱ�ֽ
        SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, WcharToChar(wallpaper_path.data()), SPIF_UPDATEINIFILE);
        return 1;
     HKEY hKey;
    DWORD dwDisposition;
    if(RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Control Panel\\Desktop"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS)
    {
        // �򿪳ɹ�
        WriteToLog("Successfully opened key!");
        std::cout << "Successfully opened key!" << std::endl;
        LPCTSTR lpValueName = TEXT("Wallpaper");
        LPCTSTR lpData = wallpaper_path.data();
        WriteToLog(WcharToChar(wallpaper_path.data()));
        std::cout << WcharToChar(wallpaper_path.data())<< std::endl;
        DWORD cbData = sizeof(TCHAR) * (_tcslen(lpData) + 1);
        if(RegSetValueEx(hKey, lpValueName, 0, REG_SZ, (BYTE*)lpData, cbData) == ERROR_SUCCESS)
        {
            // �޸ĳɹ�
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