#include "ZefraServiceToos.h"
char* m_char = NULL;  
wchar_t* m_wchar  = NULL;
void zServiceToos::DestoryPoint(void* point,bool isArr) 
{
    if(point && isArr){ delete[] point;point = NULL; }
    if(point && !isArr){ delete point;point = NULL; }
}
void inline zServiceToos::Release()  
{  
    if(m_char) {delete m_char; m_char = NULL;}  
    if(m_wchar){delete m_wchar; m_wchar = NULL;}  
}
UINT32 zServiceToos::GetRandomNumber(UINT32 min,UINT32 max) {
    return rand()%(max - min + 1) + min;
}
int zServiceToos::WriteToLog(const char* str,const char* path)
{
    FILE* fp = NULL;
    fp = fopen(path, "a+");//���ȥԤ���������������һ���������
    if (fp)
    {
        CTime ct =  CTime::GetCurrentTime();
        CString strTime  = ct.Format(L"%Y-%m-%d,%H:%M:%S");
        fprintf_s(fp,WcharToChar(strTime));
        fprintf_s(fp,"%s\n",str);//���ַ�д�뵽����ļ�����
        fclose(fp);
        return 1;
    }
    return 0;
}

wchar_t* zServiceToos::CharToWchar(const char* c)  
{  
    Release();  
    int len = MultiByteToWideChar(CP_ACP,0,c,strlen(c),NULL,0);  
    m_wchar = new wchar_t[len+1];  
    MultiByteToWideChar(CP_ACP,0,c,strlen(c),m_wchar,len);  
    m_wchar[len]='\0';  
    return m_wchar;  
} 

char* zServiceToos::WcharToChar(const wchar_t* wp)  
{  
    Release();  
    int len= WideCharToMultiByte(CP_ACP,0,wp,wcslen(wp),NULL,0,NULL,NULL);  
    m_char= new char[len+1];  
    WideCharToMultiByte(CP_ACP,0,wp,wcslen(wp),m_char,len,NULL,NULL);  
    m_char[len]='\0';  
    return m_char;  
}
char* zServiceToos::CompanChar(const char* charA,const char* charB)
{
    //malloc������ڴ���ڳ���main����������ϵͳ����
    char*chrRes = new char[strlen(charA) + strlen(charB) + 1];//��ϵͳ����ָ��a+b+1���ֽڵ��ڴ�ռ�����c��a+b+1����c������a,b�ͽ�β��'\0'
    for (int i = 0; charA[i] != '\0'; i++){
        chrRes[i] = charA[i];
    }
    for (int i = 0; charB[i] != '\0'; i++){
        chrRes[i + strlen(charA)] = charB[i];
    }
    chrRes[strlen(charA) + strlen(charB)] = '\0';
    return chrRes;

}
char*  zServiceToos::GetRootPath() {
    /*
    _pgmptr��C/C++�����е�һ��ȫ�ֱ�������λ��stdlib.hͷ�ļ��У�
    ���ڴ洢��ǰ�����ִ���ļ�������·�����ñ���������ʱ�Զ�����ʼ����
    �����ֶ��޸ġ�
    */
    char* path = _pgmptr;
    char * _running_path =  nullptr;
    for(int i = strlen(path) - 1; i >= 0 ; --i) 
    {
        if(path[i] == '\\') 
        {
            _running_path =  new char[i + 1];
            for (int j = 0; j < i; j++){
                _running_path[j] = path[j];
            }
            _running_path[i] = '\0';
            break;
        }
    }
    return _running_path;
}