#ifndef _ZEFRASERVICETOOS_
#define _ZEFRASERVICETOOS_
#include<iostream>
#include <atltime.h>
namespace zServiceToos {
	void inline Release();
	/// <summary>
	/// ����һ���ļ���������ļ���д������
	/// </summary>
	/// <param name="str">д����־����Ϣ</param>
	/// <param name="path">Ҫд����ļ�·��</param>
	/// <returns></returns>
	__declspec(dllexport) int WriteToLog(const char* str,const char* path);
	/// <summary>
	///  Wchar*תChar*
	/// </summary>
	/// <param name="w">��ת����wchar*����</param>
	/// <returns></returns>
	__declspec(dllexport) char* WcharToChar(const wchar_t* w);
	/// <summary>
	/// Char*תWchar*
	/// </summary>
	/// <param name="c">��ת����char*����</param>
	/// <returns></returns>
	__declspec(dllexport) wchar_t* CharToWchar(const char* c);
	/// <summary>
	/// ƴ��charA+charB�ַ����������ַ�C
	/// </summary>
	/// <param name="charA">�ַ�A</param>
	/// <param name="charB">�ַ�B</param>
	/// <returns></returns>
	__declspec(dllexport) char* CompanChar(const char* charA,const char* charB);
	/// <summary>
	/// ��ȡmin��max֮��������������min������max�������ֶ�����srand(time(0))��Ч
	/// </summary>
	/// <param name="min">��Сֵ</param>
	/// <param name="max">���ֵ</param>
	/// <returns></returns>
	__declspec(dllexport) UINT32 GetRandomNumber(UINT32 min,UINT32 max);
	/// <summary>
	/// ��ȡ��ǰĿ¼�ľ���·��
	/// </summary>
	/// <returns></returns>
	__declspec(dllexport) char * GetRootPath();

}

#endif 