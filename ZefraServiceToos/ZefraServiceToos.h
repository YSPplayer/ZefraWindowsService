#ifndef _ZEFRASERVICETOOS_
#define _ZEFRASERVICETOOS_
#include<iostream>
#include <atltime.h>
namespace zServiceToos {
	/// <summary>
	/// 释放我们的指针内存，如果为true则delete[]，默认delete[]
	/// </summary>
	__declspec(dllexport) void DestoryPoint(void* point,bool isArr = true);
	/// <summary>
	/// 创建一个文件，并向该文件中写入数据
	/// </summary>
	/// <param name="str">写入日志的信息</param>
	/// <param name="path">要写入的文件路径</param>
	/// <returns></returns>
	__declspec(dllexport) int WriteToLog(const char* str,const char* path);
	/// <summary>
	///  Wchar*转Char*，第二次调用时会释放掉前者调用所返回对象的内存
	/// </summary>
	/// <param name="w">被转换的wchar*对象</param>
	/// <returns></returns>
	__declspec(dllexport) char* WcharToChar(const wchar_t* w);
	/// <summary>
	/// Char*转Wchar*，第二次调用时会释放掉前者调用所返回对象的内存
	/// </summary>
	/// <param name="c">被转换的char*对象</param>
	/// <returns></returns>
	__declspec(dllexport) wchar_t* CharToWchar(const char* c);
	/// <summary>
	/// 拼接charA+charB字符，返回新字符C,，请手动释放内存
	/// </summary>
	/// <param name="charA">字符A</param>
	/// <param name="charB">字符B</param>
	/// <returns></returns>
	__declspec(dllexport) char* CompanChar(const char* charA,const char* charB);
	/// <summary>
	/// 获取min和max之间的随机数，包括min不包括max，必须手动调用srand(time(0))生效
	/// </summary>
	/// <param name="min">最小值</param>
	/// <param name="max">最大值</param>
	/// <returns></returns>
	__declspec(dllexport) UINT32 GetRandomNumber(UINT32 min,UINT32 max);
	/// <summary>
	/// 获取当前目录的绝对路径，请手动释放内存
	/// </summary>
	/// <returns></returns>
	__declspec(dllexport) char * GetRootPath();
	void inline Release();

}

#endif 