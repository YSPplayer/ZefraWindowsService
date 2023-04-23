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
    ��������Ҫ�Ȳ�ѯ���ǵķ������Ƿ��д������ǵ�������û�л������ͨ��״̬��
    ���Ǿ͹رյ��ý���
    */
    SC_HANDLE scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scmHandle == NULL) {
        // ��SCM���ݿ�ʧ��,���������Ϣ��ͣ�ý���
        zServiceToos::WriteToLog("��exe��Failed to open SCM database!",log_path);
        return false;
    }
    SC_HANDLE serviceHandle = OpenService(scmHandle, IPSERVICENAME, SERVICE_QUERY_CONFIG);
    if (serviceHandle == NULL) {
        // ��ָ�����Ƶķ���ʧ�ܣ����������Ϣ
        zServiceToos::WriteToLog("��exe��Failed to open service!",log_path);
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
            zServiceToos::WriteToLog(res,log_path);
            CloseServiceHandle(serviceHandle); //�رշ�����
            CloseServiceHandle(scmHandle); // �ر�SCM���ݿ���
            delete[] res;
            res = NULL;
            return false;
        } else {
            //��ʾ����������Ĵ���
            zServiceToos::WriteToLog("��exe��Failed query service becaseof cache overflow,will retry",log_path);
            LPQUERY_SERVICE_CONFIG lpqscBuffer = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LPTR, dwBytesNeeded);
            if (lpqscBuffer == nullptr) {
                zServiceToos::WriteToLog("��exe��LocalAlloc failed",log_path);
                CloseServiceHandle(serviceHandle); //�رշ�����
                CloseServiceHandle(scmHandle); // �ر�SCM���ݿ���
                return false;
            }
            if (!QueryServiceConfig(serviceHandle, lpqscBuffer, dwBytesNeeded, &dwBytesNeeded)) {
                zServiceToos::WriteToLog("��exe��Failed query with overflow retry",log_path);
                LocalFree(lpqscBuffer);
                CloseServiceHandle(serviceHandle);
                CloseServiceHandle(scmHandle); 
                return false;
            }
            zServiceToos::WriteToLog("��exe��success query with overflow retry",log_path);
            if (lpqscBuffer && (lpqscBuffer->dwStartType == SERVICE_DISABLED 
                || lpqscBuffer->dwStartType == SERVICE_SYSTEM_START)) {
                zServiceToos::WriteToLog("��exe��Service is disabled.",log_path);
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

        // �жϷ����Ƿ�������״̬
        if (serviceConfig.dwStartType == SERVICE_DISABLED || serviceConfig.dwStartType == SERVICE_SYSTEM_START) {
            zServiceToos::WriteToLog("��exe��Service is disabled.",log_path);
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
    //10���ӻ�һ�α�ֽ
    #define SLEEP_TIME 600000
    char* running_path = zServiceToos::GetRootPath();//��ȡ�ļ�������Ŀ¼
    char* log_path = zServiceToos::CompanChar(running_path,"\\log\\run.log");
    char* pics_path = zServiceToos::CompanChar(running_path,"\\pics\\");
    char *before_path = zServiceToos::CompanChar(running_path,"\\pics\\*");
    const wchar_t* folder_path = zServiceToos::CharToWchar(before_path);//���ǲ����ļ���·��
    WIN32_FIND_DATAW file_data;//���� WIN32_FIND_DATAW �ṹ�壬���ڱ�����ҵ����ļ���Ϣ
    delete[] before_path;//�ͷ��ڴ�
    before_path = NULL;
    HANDLE file_handle = FindFirstFileW(folder_path, &file_data);//���ҵ�һ��ƥ����ļ�
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        //������ƥ�䲻���ļ���
        zServiceToos::WriteToLog("��exe��WIN32_FIND_DATAW Failed to open folder!",log_path);
        return 0;
    }
    //����һ��vector�������洢���ǵ�����
    std::vector<std::wstring> V;
    do
    {
        // ������ļ����ǲ���Ҫ
        if (!(file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            std::wstring fileName = file_data.cFileName;
            std::wstring fileExt = fileName.substr(fileName.rfind(L".") + 1);
            if (fileExt == L"png" || fileExt == L"jpg") {
                //�洢���ǵ�ֵ
                V.push_back(file_data.cFileName);
            }
        }
    } while (FindNextFileW(file_handle, &file_data));
    //�رղ���
    FindClose(file_handle);
    srand(time(0));//�������������
    while (true)
    {
        //������Ƿ���رյĻ����˳�����
        if(!CheckService(log_path)) break;
        UINT32 index = zServiceToos::GetRandomNumber(0,V.size());
        if(index >= V.size() || index < 0) index = 0;
        char* resPath = zServiceToos::CompanChar(pics_path,zServiceToos::WcharToChar(V.at(index).data()));
        char *log = zServiceToos::CompanChar("��exe��load pics from path:",resPath);
        zServiceToos::WriteToLog(log,log_path);//д���·��
        std::wstring wallpaper_path = zServiceToos::CharToWchar(resPath);
        //��ӱ�ֽ
        if(!SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, zServiceToos::WcharToChar(wallpaper_path.data()), SPIF_UPDATEINIFILE)) 
        {
            zServiceToos::WriteToLog("��exe��SystemParametersInfoA error to change pics",log_path);
            return 0;
        } 
        zServiceToos::WriteToLog("��exe��SystemParametersInfoA successly to change pics",log_path);
        delete[] resPath;//�ͷ��ڴ�
        resPath = NULL;
        delete[] log;
        log = NULL;
        Sleep(SLEEP_TIME);
    }
    
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