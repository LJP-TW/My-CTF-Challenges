#include <Windows.h>

#include "APIfuncpointer.h"
#include "resolver.h"
#include "stubber.h"

#define PROCESS_CREATE_FLAGS_PROTECTED_PROCESS 0x00000040

#define THREAD_CREATE_FLAGS_CREATE_SUSPENDED 0x00000001

typedef enum _PS_PROTECTED_TYPE {
    PsProtectedTypeNone,
    PsProtectedTypeProtectedLight,
    PsProtectedTypeProtected,
    PsProtectedTypeMax
} PS_PROTECTED_TYPE;

typedef enum _PS_PROTECTED_SIGNER {
    PsProtectedSignerNone,
    PsProtectedSignerAuthenticode,
    PsProtectedSignerCodeGen,
    PsProtectedSignerAntimalware,
    PsProtectedSignerLsa,
    PsProtectedSignerWindows,
    PsProtectedSignerWinTcb,
    PsProtectedSignerMax
} PS_PROTECTED_SIGNER;

typedef struct _PS_PROTECTION {
    union {
        UCHAR Level;
        struct {
            UCHAR Type : 3;
            UCHAR Audit : 1;
            UCHAR Signer : 4;
        };
    };
} PS_PROTECTION, * PPS_PROTECTION;

char stubs[0x10][STUBSIZE];

void GetAPIStub(int idx, BYTE* imagebase, CHAR* procname)
{
    void* proc = GetProcAddr(imagebase, procname);

    for (DWORD i = 0; i < STUBSIZE; ++i) {
        // Check for int 0x3
        if (((BYTE*)proc)[i] == 0xcc) {
            exit();
        }
    }

    memcpy(stubs[idx], proc, STUBSIZE);
}

void exit(void)
{
    ((__ZwTerminateProcess)(char*)stubs[ID_ZwTerminateProcess])();
}

/*
 *  Ref: https://github.com/Microwave89/createuserprocess
 *  Example usage:
 *    STARTUPINFO si;
 *    PROCESS_INFORMATION pi;
 * 
 *    WCHAR imageName[] = L"\\??\\C:\\Windows\\System32\\calc.exe";
 *    UNICODE_STRING uImageName;
 *    uImageName.Buffer = imageName;
 *    uImageName.Length = sizeof(imageName) - sizeof(UNICODE_NULL);
 *    uImageName.MaximumLength = sizeof(imageName);
 *    WrapNtCreateUserProcess(&uImageName);
*/
NTSTATUS WrapNtCreateUserProcess(PUNICODE_STRING pProcessImageName) 
{
    PS_CREATE_INFO procInfo;
    REAL_RTL_USER_PROCESS_PARAMETERS userParams;
    PS_ATTRIBUTE_LIST attrList;
    NTSTATUS status;
    HANDLE hProcess = NULL;
    HANDLE hThread = NULL;

    if (pProcessImageName) {
        BYTE* peb = (BYTE*)__readgsqword(0x60);
        BYTE* curProcessParameters = *(BYTE**)(peb + 0x20);

        memset(&userParams, 0, sizeof(RTL_USER_PROCESS_PARAMETERS));
        memset(&attrList, 0, sizeof(PS_ATTRIBUTE_LIST) - sizeof(PS_ATTRIBUTE));
        memset(&procInfo, 0, sizeof(PS_CREATE_INFO));

        procInfo.Size = sizeof(PS_CREATE_INFO);

        attrList.TotalLength = sizeof(PS_ATTRIBUTE_LIST) - sizeof(PS_ATTRIBUTE);
        attrList.Attributes[0].Attribute = PsAttributeValue(PsAttributeImageName, FALSE, TRUE, FALSE);
        attrList.Attributes[0].Size = pProcessImageName->Length;
        attrList.Attributes[0].Value = (ULONG_PTR)pProcessImageName->Buffer;

        status = ((__NtCreateUserProcess)(char*)stubs[ID_NtCreateUserProcess])(
            &hProcess,
            &hThread,
            MAXIMUM_ALLOWED,
            MAXIMUM_ALLOWED,
            NULL,
            NULL,
            NULL,
            NULL,
            (PREAL_RTL_USER_PROCESS_PARAMETERS)curProcessParameters,
            &procInfo,
            &attrList);
    }
    else {
        status = STATUS_INVALID_PARAMETER;
    }

    return status;
}

void WrapSleep(DWORD sec)
{
    LARGE_INTEGER counter, frequency;
    DWORD64 curtime;

    ((__NtQueryPerformanceCounter)(char*)stubs[ID_NtQueryPerformanceCounter])(&counter, &frequency);

    curtime = counter.QuadPart;

    while ((counter.QuadPart - curtime) / 10000000 < sec) {
        ((__NtQueryPerformanceCounter)(char*)stubs[ID_NtQueryPerformanceCounter])(&counter, &frequency);
    }
}

// Ref: https://www.unknowncheats.me/forum/c-and-c-/467608-afd-socket.html#post3230075
void WrapRecv(DWORD socket, BYTE* buf, DWORD len, DWORD flags)
{
    typedef struct _AFD_RECV_INFO
    {
        MY_PWSABUF BufferArray;
        ULONG BufferCount;
        ULONG AfdFlags;
        ULONG TdiFlags;
    }AFD_RECV_INFO, * PAFD_RECV_INFO;

    HANDLE Event;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS status = SC_NtCreateEvent(&Event, EVENT_ALL_ACCESS, NULL, SynchronizationEvent, FALSE);

    if (NT_SUCCESS(status))
    {
        MY_WSABUF buff = { len, buf };
        AFD_RECV_INFO Input = { &buff, 1, 8, TDI_RECEIVE_NORMAL };

        status = SC_NtDeviceIoControlFile(
            (HANDLE)socket, 
            Event, 
            NULL, 
            NULL, 
            &IoStatusBlock, 
            IOCTL_AFD_RECV, 
            &Input, 
            sizeof(Input), 
            NULL, 
            NULL);

        if (status == STATUS_PENDING)
        {
            SC_NtWaitForSingleObject(Event, TRUE, 0);
            status = IoStatusBlock.Status;
        }

        SC_NtClose(Event);
    }
}
