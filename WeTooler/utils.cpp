#pragma once
#include "utils.h"
string writeToTempFile(const string dir, const string prefix, const string suffix, const char* ptrData, const int dataLen) {
	int rand = (int)GetTickCount64();
	string filePath = dir;
	filePath += "\\";
	filePath += prefix;
	filePath += std::to_string(rand);
	filePath += suffix;
	HANDLE hFile = CreateFileA(filePath.data(), GENERIC_ALL, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD realWrite;
		WriteFile(hFile, ptrData, dataLen, &realWrite, 0);
		CloseHandle(hFile);
	}
	return filePath;
}

string decryptImg(const string imgPath, char key) {

	HANDLE hFile = CreateFileA(imgPath.data(), GENERIC_ALL, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);


	int dataLen = 0;
	dataLen = GetFileSize(hFile, 0);

	if (dataLen>0 && hFile != INVALID_HANDLE_VALUE) {
		char* buff = new char[dataLen]();
		DWORD realRead;
		ReadFile(hFile, buff, dataLen, &realRead, 0);
		CloseHandle(hFile);
		if (realRead > 0) {
			int i = 0;
			for (i = 0; i < dataLen; i += 4) {
				buff[i] = buff[i] ^ key;
			}
			if (i != dataLen) {
				int tm = dataLen - i;
				int ls;
				memcpy(&ls, &buff[i], tm);
				ls = ls ^ key;
				memcpy(&buff[i], &ls, tm);
			}
		}
		string newImagePath = imgPath + ".jpg";

		hFile = CreateFileA(newImagePath.data(), GENERIC_ALL, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile != INVALID_HANDLE_VALUE) {
			DWORD realWrite;
			WriteFile(hFile, buff, dataLen, &realWrite, 0);
			CloseHandle(hFile);
		}
		delete []buff;
		return newImagePath;

	}
	return {};
}

//wstring=>string
std::string WString2String(const std::wstring& ws)
{
	std::string strLocale = setlocale(LC_ALL, "");
	const wchar_t* wchSrc = ws.c_str();
	size_t nDestSize = wcstombs(NULL, wchSrc, 0) + 1;
	char* chDest = new char[nDestSize];
	memset(chDest, 0, nDestSize);
	wcstombs(chDest, wchSrc, nDestSize);
	std::string strResult = chDest;
	delete[]chDest;
	setlocale(LC_ALL, strLocale.c_str());
	return strResult;
}
// string => wstring
std::wstring String2WString(const std::string& s)
{
	std::string strLocale = setlocale(LC_ALL, "");
	const char* chSrc = s.c_str();
	size_t nDestSize = mbstowcs(NULL, chSrc, 0) + 1;
	wchar_t* wchDest = new wchar_t[nDestSize];
	wmemset(wchDest, 0, nDestSize);
	mbstowcs(wchDest, chSrc, nDestSize);
	std::wstring wstrResult = wchDest;
	delete[]wchDest;
	setlocale(LC_ALL, strLocale.c_str());
	return wstrResult;
}


bool __stdcall canReadWrite(LPVOID address) {
	MEMORY_BASIC_INFORMATION mem;
	VirtualQuery(address, &mem, sizeof(MEMORY_BASIC_INFORMATION));
	if (mem.AllocationProtect == PAGE_EXECUTE_READWRITE || mem.AllocationProtect == PAGE_READWRITE || mem.AllocationProtect == PAGE_READONLY) {
		return true;
	}
	return false;
}