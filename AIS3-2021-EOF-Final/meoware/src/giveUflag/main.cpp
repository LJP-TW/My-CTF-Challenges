#define _WINSOCKAPI_ 
#include <Windows.h>

#include <vector>
#include <stdio.h>

#include "strobf.hpp"
#include "crc32.h"
#include "aes.h"
#include "../meoware/stubber.h"
#include "../meoware/meoware.h"

meow_context* context;
WrapFuncs* funcs;

using namespace std;

constexpr auto str_aes_key = OBFUSCATE("EOFEOFEOFEOFEOFE");

void NTAPI TLS_init(PVOID DllHandle, DWORD Reason, PVOID Reserved);

#pragma comment (linker, "/INCLUDE:_tls_used")
#pragma comment (linker, "/INCLUDE:p_tls_array")
#pragma const_seg(".CRT$XLA")
EXTERN_C const PIMAGE_TLS_CALLBACK p_tls_array[] = { TLS_init, 0 };
#pragma const_seg()

void NTAPI TLS_init(PVOID DllHandle, DWORD Reason, PVOID pReserved)
{
    context = (meow_context*)pReserved;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved)
{
    BOOL bReturnValue = TRUE;
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        funcs = (WrapFuncs*)lpReserved;
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return bReturnValue;
}

void run(void)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char cmd[0x100];
    WCHAR imageName[0x200];
    UNICODE_STRING uImageName;
    DWORD i;

    funcs->WrapRecv(context->sock, (BYTE*)cmd, sizeof(cmd), 0);

    for (i = 0; i < 0x100; ++i) {
        imageName[i] = cmd[i];

        if (cmd[i] == 0) {
            break;
        }
    }

    uImageName.Buffer = imageName;
    uImageName.Length = (i + 1) * 2 - sizeof(UNICODE_NULL);
    uImageName.MaximumLength = (i + 1) * 2;
    funcs->WrapNtCreateUserProcess(&uImageName);
}

void eof(void)
{
    DEOBFUSCA(str_aes_key);
    
    for (int i = 0;; ++i) {
        if (!((char*)&str_aes_key)[i]) {
            break;
        }

        context->aes_key.push_back(((char*)&str_aes_key)[i]);
    }
}

extern "C" __declspec(dllexport) DWORD Meow(DWORD sec)
{
    char cmd[4];

    while (true) {
        funcs->WrapRecv(context->sock, (BYTE*)cmd, sizeof(cmd), 0);

        if (xcrc32((BYTE*)cmd, sizeof(cmd), 0) == -639700292) {
            // RUN
            funcs->WrapSleep(sec);
            run();
        }
        else if (xcrc32((BYTE*)cmd, sizeof(cmd), 0) == -540142150) {
            // BYE
            funcs->exit();
        }
        else if (xcrc32((BYTE*)cmd, sizeof(cmd), 0) == -1459710112) {
            // EOF
            funcs->WrapSleep(sec);
            eof();
        }
        else if (xcrc32((BYTE*)cmd, sizeof(cmd), 0) == -21005182) {
            // RET
            break;
        }
        else if (xcrc32((BYTE*)cmd, sizeof(cmd), 0) == 1540589298) {
            // WRY
            funcs->WrapSleep(sec);
            context->meow_setenc(context);
        }
    }

    return 0;
}

extern "C" __declspec(dllexport) DWORD Weom(DWORD _)
{
    constexpr auto congratulations = OBFUSCATE("Congratulations!");

    AES aes(AESKeyLength::AES_128);
    vector<unsigned char> plain = aes.DecryptECB(context->aes_enc, context->aes_key);
    string s(plain.begin(), plain.end());

    if (xcrc32((BYTE*)(s.c_str()), 44, 0) == -1156153501) {
        DEOBFUSCA(congratulations);
        cout << (char*)&congratulations << endl;
    }

    return xcrc32((BYTE*)(s.c_str()), 44, 0);
}