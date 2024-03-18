#include <windows.h>
#include <WinBase.h>
#include <cstdio>
#include <atlstr.h>
#include "resource.h"
#include <clocale>
int main(int argc, const char* argv)
//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpCmdLine, int nCmdShow)
 {
	//setlocale(LC_ALL, "zh-CN.UTF-8");
	HINSTANCE hInstance = GetModuleHandle(NULL);
	
	auto name=MAKEINTRESOURCE(IDR_FILE1);
	HRSRC res= FindResource(hInstance, name, TEXT("FILE"));
	if (res == nullptr) {
		printf("未找到资源！Error:%d\n", GetLastError());
		return 0;
	}
	auto lres=LoadResource(hInstance, res);
	if (lres == nullptr) {
		printf("加载资源失败！");
		return 0;
	}
	auto lpResLock = LockResource(lres);
	if (lpResLock == NULL)
	{
		printf("获取资源地址失败！");
		return 0;
	}
	char* str = (char*)lpResLock;
	printf("\n字符串：%s\n", str);
	return 0;
}