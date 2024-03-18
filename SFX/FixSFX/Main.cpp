#include <windows.h>
#include <WinBase.h>
#include <cstdio>
int main(int argc, const char* argv) {
	
	auto hUpdateRes = BeginUpdateResource(TEXT("SFX.exe"), FALSE);
	if (hUpdateRes == NULL)
	{
		printf("加载文件失败！\n");
		return 0;
	}
	char info[] = "\n-------------------------------\n\t\t\tsfx-fix\n-----------------------------------\n";

	auto result = UpdateResource(hUpdateRes,    // update resource handle
		TEXT("FILE"),                         // change dialog box resource
		MAKEINTRESOURCE(102),         // dialog box id
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),  // neutral language
		info,                         // ptr to resource info
		sizeof(info));       // size of resource info

	if (result == FALSE)
	{
		printf("Could not add resource.");
		return 0;
	}

	// Write changes to FOOT.EXE and then close it.
	if (!EndUpdateResource(hUpdateRes, FALSE))
	{
		printf("Could not write changes to file.");
		return 0;
	}
	return 0;
}