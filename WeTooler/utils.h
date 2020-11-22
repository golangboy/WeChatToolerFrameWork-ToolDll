#pragma once
#include <Windows.h>
#include <string>
#include <stdlib.h>
#include <locale.h>
#include <map>
using namespace std;
string decryptImg(const string imgPath,  char key);
string writeToTempFile(const string dir, const string prefix, const string suffix, const char* ptrData, const int dataLen);
std::string WString2String(const std::wstring& ws);
std::wstring String2WString(const std::string& s);
bool __stdcall canReadWrite(LPVOID address);