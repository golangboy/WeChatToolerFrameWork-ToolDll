#include <Windows.h>
#include "detours.h"
#include "utils.h"
#pragma comment(lib,"detours.lib")
#pragma warning(disable:4996)


HMODULE weChatWin = 0;
DWORD dwHookedImageHelpFunc = 0;

typedef struct {
	DWORD srcFunction;
	DWORD userFunction;
}CallBack;

/// <summary>
/// 各种消息回调函数
/// </summary>
typedef void(__stdcall* dstImageFunc)(const wchar_t*wxId, const wchar_t*imgPath);
typedef void(__stdcall* dstVoiceFunc)(const wchar_t* wxId, const char*ptrData,const int dataLen);
typedef void(__stdcall* dstTextFunc)(const wchar_t* wxId, const wchar_t* msg,const int type);

CallBack callBack_ImageMessage, callBack_TextMessage, callBack_VoiceMessage;




void setCallBack_ImageMessage(dstImageFunc* callBackFunction)
{
	callBack_ImageMessage.userFunction = (DWORD)callBackFunction;
}

void setCallBack_VoiceMessage(dstVoiceFunc callBackFunction)
{
	callBack_VoiceMessage.userFunction = (DWORD)callBackFunction;
}

void setCallBack_TextMessage(dstTextFunc callBackFunction) {
	callBack_TextMessage.userFunction = (DWORD)callBackFunction;
}



void __stdcall Call_SendFileMessage(wchar_t* wxid,  wchar_t* imgPath) {
	struct UnicodeStruct
	{
		wchar_t* ptr;
		int len;
	};
	UnicodeStruct* id = new UnicodeStruct();
	id->ptr = wxid;
	id->len = lstrlenW(wxid);
	UnicodeStruct* msg = new UnicodeStruct();
	msg->ptr = imgPath;
	msg->len = lstrlenW(imgPath);
	char* buff = new char[10000]();
	int function = 0x2E6F90 + (int)weChatWin;
	int constNumber = *((int*)(0x2E46B8 + (int)weChatWin));
	__asm {

		pushad;
		pushfd;

		push msg;
		push msg;
		push id;
		mov ecx, constNumber;
		push buff;
		call function

		popfd;
		popad;


	}

	delete[]buff;
	delete id;
	delete msg;
}

/*
void Call_SendPicMessageRight(const wstring wxid, const wstring imgPath) {
	struct UnicodeStruct
	{
		wchar_t* ptr;
		int len;
	};
	UnicodeStruct* id = new UnicodeStruct();
	id->ptr = (wchar_t*)wxid.data();
	id->len = wxid.size();
	UnicodeStruct* msg = new UnicodeStruct();
	msg->ptr = (wchar_t*)imgPath.data();
	msg->len = imgPath.size();
	char* buff = new char[50000]();
	int function = 0x2E6F90 + (int)weChatWin;
	int constNumber = *((int*)(0x2E46B8 + (int)weChatWin));
	__asm {
		pushad;
		pushfd;

		push msg;
		push msg;
		push id;
		mov ecx, constNumber;
		push buff;
		call function

			popfd;
		popad;
	}

	delete[]buff;
}
*/

void __stdcall Call_SendTextMessage(wchar_t* wxid, wchar_t* msg)
{

	struct UnicodeStruct
	{
		wchar_t* ptr;
		int len;
	};
	UnicodeStruct* id = new UnicodeStruct();
	id->ptr = wxid;
	id->len = lstrlenW(wxid);
	UnicodeStruct* text = new UnicodeStruct();
	text->ptr = msg;
	text->len = lstrlenW(msg);
	char buff[4000]{ 0 };
	int function = 0x2EB4E0 + (int)weChatWin;
	__asm {
		pushad;
		pushfd;

		mov edx, id;
		push 1;
		push 0;
		push text;
		lea ecx, buff;
		call function;
		add esp, 0xc;

		popfd;
		popad;
	}
	delete id;
	delete msg;
		
}



map<string, string> wxidImgPath;
void __stdcall Mid_beHook_ImageMessage(int ptrData, int len,int ptr)
{
	//ptr -> img_path(unicode)
	//ptr+4 path_len
	if (0 == canReadWrite((LPVOID)ptr))
		return;

	wchar_t* imgPath = (wchar_t*)(*((int*)(ptr)));
	//char* dataPtr = (char*)(*((int*)(ptr+0x28)));
	//int *ptrdataLen = (int*)(*((int*)(ptr+0x28+0x4)));
	string sImgPath = WString2String(imgPath);
	if (wxidImgPath.find(sImgPath) != wxidImgPath.end()) {
		//MessageBoxA(0, sImgPath.data(), wxidImgPath[sImgPath].data(), 0);
		string imgPath = writeToTempFile("Img", "Img_", ".jpg", (char*)ptrData, len);
		//Call_SendFileMessage( String2WString(wxidImgPath[sImgPath]) , String2WString(imgPath));

		if (callBack_ImageMessage.userFunction != 0) {
			char buff[MAX_PATH];
			GetCurrentDirectoryA(MAX_PATH, buff);
			string tmp = string(buff) + "\\";
			tmp += imgPath;
			((dstImageFunc)(callBack_ImageMessage.userFunction))(String2WString(wxidImgPath[sImgPath]).data(),   String2WString(tmp).data());
		}
		wxidImgPath.erase(sImgPath);
	}
	
}

__declspec(naked) int Mid_beHook_ImageMessage_Help()
{
	//ecx -> img_path(unicode)
	//ecx+4 path_len

	//ecx+0x28 -> data
	//ecx+0x28+4 -> data_len

	__asm {
		push ebp
		mov ebp,esp

		pushad
		pushfd

		push ecx
		mov eax,esp
		add eax,48
		push [eax]
		push edi
		call Mid_beHook_ImageMessage

		popfd
		popad

		mov esp,ebp
		pop ebp

		jmp dwHookedImageHelpFunc
		//ret
	}

}
void __stdcall Mid_beHook_ImageMessage_Help2(wchar_t* imgPath,wchar_t* wxid) {
	if (canReadWrite(imgPath) && canReadWrite(wxid)) {
		wxidImgPath[WString2String(imgPath)] = WString2String(wxid);
	}
}
__declspec(naked) void  Mid_beHook_ImageMessage_Jmp(int* ptrUnicodeWxid, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int* ptrUnicodeImgPath)
{
	_asm {
		push ebp
		mov ebp, esp

		pushad
		pushfd

		mov eax,esp
		add eax,11*4
		push [eax]
		add eax,13*4
		push[eax]
		call Mid_beHook_ImageMessage_Help2



		popfd
		popad

		mov esp, ebp
		pop ebp

		jmp callBack_ImageMessage.srcFunction
	}

	//MessageBoxW(0, (wchar_t*)ptrUnicodeImgPath,(wchar_t*)ptrUnicodeWxid, 0);
	//char* tempFile = writeToTempFile("img","img_",);
	//wxidImgPath[WString2String((wchar_t*)ptrUnicodeImgPath)]= WString2String((wchar_t*)ptrUnicodeWxid);
	//char key = *((char*)weChatWin+0x126E179);

	//string newImgPath = decryptImg(WString2String((wchar_t*)ptrUnicodeImgPath),key);

	//typedef void (*src)();
	//((src)callBack_ImageMessage.srcFunction)();

}





void __stdcall  Mid_beHook_VoiceMessage(char* ptrVoiceData, wchar_t* wxId, int len)
{
	if (callBack_VoiceMessage.userFunction != 0) {
		((dstVoiceFunc)(callBack_VoiceMessage.userFunction))(wxId, ptrVoiceData,len);
	}

	string tempFile = writeToTempFile(string("Voice"), string("Voice_"), ".mp3", (const char*)ptrVoiceData, len);
	string command = (string("decode ") + tempFile + " " + tempFile + ".mp3");
	system(command.data());
	command = (string("lamp ") + tempFile + ".mp3");
	system(command.data());
}
void __stdcall Mid_beHook_VoiceMessage_Jmp_Help(int a) {
	int ptr_1 = ((int*)a)[46];
	ptr_1 = ptr_1 - 0x58 + 0x20;
	if (canReadWrite((LPVOID)ptr_1))
	{
		ptr_1 = *((int*)ptr_1);
		if (canReadWrite(((int*)(ptr_1 + 4))))
		{
			ptr_1 = *((int*)(ptr_1 + 4));
			if (canReadWrite((LPVOID)ptr_1)) {
				ptr_1 = *((int*)ptr_1);

				int dataLen = ((int*)a)[3];
				wchar_t* wxidUnicode = (wchar_t*)((int*)a)[15];

				//char buff[200];
				//itoa(dataLen, buff, 10);
				//MessageBoxA(0, (char*)ptr_1, buff, 0);
				//MessageBoxW(0, wxidUnicode, 0, 0);
				Mid_beHook_VoiceMessage((char*)ptr_1, wxidUnicode, dataLen);
			}
		}
	}
	/*
	ptr_1 = *((int*)ptr_1);
	ptr_1 = *((int*)(ptr_1 + 4));
	ptr_1 = *((int*)ptr_1);
	MessageBoxA(0, 0, (char*)ptr_1, 0);
	*/
}
__declspec(naked)  void Mid_beHook_VoiceMessage_Jmp(int a)
{
	//((int*)&a)[3];  voice_len
	//((int*)&a)[15]; wxid
	//((int*)&a)[30]; src
	//((int*)&a)[46]; ptr

	//  [[[ ptr-0x58+0x20 ] + 0x4]] = SKIL语音数据
	// ptr_1就是指向SILK语音格式的指针

	//int ptr_1 = ((int*)&a)[46];
	//ptr_1 = ptr_1 - 0x58 + 0x20;

	__asm {
		push ebp
		mov ebp, esp

		pushad
		pushfd
		mov eax, esp
		add eax, 8 + 24 + 12
		push eax
		call Mid_beHook_VoiceMessage_Jmp_Help
		popfd
		popad;

		mov esp, ebp
			pop ebp
			jmp callBack_VoiceMessage.srcFunction
	}

	//__asm pushad;
	//__asm pushfd;

	/*
	if (canReadWrite((LPVOID)ptr_1))
	{
		ptr_1 = *((int*)ptr_1);
		if (canReadWrite(((int*)(ptr_1 + 4))))
		{
			ptr_1 = *((int*)(ptr_1 + 4));
			if (canReadWrite((LPVOID)ptr_1)){
				ptr_1 = *((int*)ptr_1);
				MessageBoxA(0, (char*)ptr_1, 0, 0);
			}
		}
	}

	MessageBoxA(0, 0, 0, ptr_1);

	__asm popfd;
	__asm popad;
	//__asm jmp callBack_VoiceMessage.srcFunction
	typedef void (*src)();
	((src)callBack_VoiceMessage.srcFunction)();
	*/

	/*
	__asm pushad
	__asm pushfd



	/*
	_asm
	{
		push ebp
		mov ebp,esp


		pushad
		pushfd

		mov eax, esp
		mov ebx,eax
		add ebx,0xe4
		add eax,0x38
		push [eax]			//len
		add eax,0X30

		push [eax]			//wxid
		//add eax, 0x7c
		//mov ebx,[eax]

		sub ebx,0x38
		mov ebx,[ebx]
		add ebx,4
		mov ebx, [ebx]
		mov ebx, [ebx]
		push ebx
		call Mid_beHook_VoiceMessage







		popfd
		popad

		mov esp,ebp
		pop ebp
		jmp callBack_VoiceMessage.srcFunction
	}
	*/


	//string tempFile = writeToTempFile(string("Voice"), string("Voice_"),".mp3", (const char*)ptr_1, voirce_len);
	//system(  (string("decode ") + tempFile + " " + tempFile+".mp3") .data());
	//system(  (string("lamp ")+tempFile+".mp3").data());


	//MessageBoxW(0, (wchar_t*)((int*)&a)[29], (wchar_t*)((int*)&a)[15], 0);
	//char buff[200],buff2[200];
	//itoa(ptr_1, buff, 16);
	//itoa(voirce_len, buff2, 16);
	//MessageBoxA(0, buff, buff2, 0);


	//__asm popfd;
	//__asm popad;
	//typedef void (*src)();
	//((src)callBack_VoiceMessage.srcFunction)();

}


void __stdcall Mid_beHook_TextMessage(const wstring wxid, const wstring msg,const int type) {
	//Call_SendTextMessage(  L"filehelper",L"filehelper");
	//Call_SendPicMessageRight((wchar_t*)L"filehelper", (wchar_t*)L"d:/4.png");
	if (callBack_TextMessage.userFunction != 0) {
		//MessageBoxW(0, msg.data(), 0, 0);
		((dstTextFunc)(callBack_TextMessage.userFunction))(wxid.data(), msg.data(),type);
	}
}
int __stdcall Mid_beHook_TextMessage_Help(int a, int b, int c) {
	int* e;
	__asm {
		mov e, ebx
	}
	if (canReadWrite(e)) {
		Mid_beHook_TextMessage((LPCWSTR)(*(int*)(*e + 0x40)), (LPCWSTR)(*(int*)(  (int)(*e) + 0x68)), (*((int*)((int)(*e) +0x30))));
	}

	typedef int(__stdcall* ptrSrc)(int, int, int);
	return ((ptrSrc)callBack_TextMessage.srcFunction)(a, b, c);
}


void Hook()
{
	weChatWin = LoadLibraryA("WeChatWin.dll");
	if (weChatWin != 0) {

		
		MessageBoxA(0, "Hook", 0, 0);
		
		callBack_ImageMessage.srcFunction = (DWORD)weChatWin + 0x47A010;
		DetourRestoreAfterWith();
		DetourTransactionBegin();

		DetourUpdateThread(GetCurrentThread());

		DetourAttach((PVOID*)&callBack_ImageMessage.srcFunction, (PVOID)Mid_beHook_ImageMessage_Jmp);
		DetourTransactionCommit();



		dwHookedImageHelpFunc = (DWORD)weChatWin + 0x45B440;
		DetourRestoreAfterWith();
		DetourTransactionBegin();

		DetourUpdateThread(GetCurrentThread());
		DetourAttach((PVOID*)&dwHookedImageHelpFunc, (PVOID)Mid_beHook_ImageMessage_Help);
		DetourTransactionCommit();
		
		

		
	}
	if (weChatWin != 0) {
		
		callBack_VoiceMessage.srcFunction = (DWORD)weChatWin + 0xC9760;
		DetourRestoreAfterWith();
		DetourTransactionBegin();

		DetourUpdateThread(GetCurrentThread());

		DetourAttach((PVOID*)&callBack_VoiceMessage.srcFunction, (PVOID)Mid_beHook_VoiceMessage_Jmp);
		DetourTransactionCommit();
	}

	if (weChatWin != 0) {
		callBack_TextMessage.srcFunction = (int)weChatWin + 0x2CAEE0;;
		DetourRestoreAfterWith();
		DetourTransactionBegin();

		DetourUpdateThread(GetCurrentThread());

		DetourAttach(&(PVOID&)callBack_TextMessage.srcFunction, Mid_beHook_TextMessage_Help);
		DetourTransactionCommit();
	}
}
void Enter() {}
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Hook();
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

