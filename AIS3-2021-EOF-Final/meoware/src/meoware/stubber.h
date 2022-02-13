#pragma once
#include <winternl.h>

#include "APIfuncpointer.h"

#define STUBSIZE 0x15

#pragma section(".stub", read, write, execute)
__declspec(allocate(".stub")) extern char stubs[0x10][STUBSIZE];

enum {
    ID_NtOpenProcess = 0,
    ID_ZwCreateProcess,
    ID_NtCreateUserProcess,
    ID_ZwTerminateProcess,
    ID_NtQueryPerformanceCounter,
    ID_NtDeviceIoControlFile,
    ID_NtCreateEvent,
    ID_NtWaitForSingleObject,
    ID_NtClose,
};

void GetAPIStub(int idx, BYTE* imagebase, CHAR* procname);

typedef void(*EXIT)(void);
void exit(void);

typedef NTSTATUS(*WRAPNTCREATEUSERPROCESS)(PUNICODE_STRING pProcessImageName);
NTSTATUS WrapNtCreateUserProcess(PUNICODE_STRING pProcessImageName);

typedef void(*WRAPSLEEP)(DWORD sec);
void WrapSleep(DWORD sec);

typedef void(*WRAPRECV)(DWORD socket, BYTE* buf, DWORD len, DWORD flags);
void WrapRecv(DWORD socket, BYTE* buf, DWORD len, DWORD flags);

#define SC_NtCreateEvent(EventHandle, \
                         DesiredAccess, \
                         ObjectAttributes, \
                         EventType, \
                         InitialState) \
    ((__NtCreateEvent)(char*)stubs[ID_NtCreateEvent])( \
        EventHandle, \
        DesiredAccess, \
        ObjectAttributes, \
        EventType, \
        InitialState)

#define SC_NtDeviceIoControlFile(FileHandle, \
                                 Event, \
                                 ApcRoutine, \
                                 ApcContext, \
                                 IoStatusBlock, \
                                 IoControlCode, \
                                 InputBuffer, \
                                 InputBufferLength, \
                                 OutputBuffer, \
                                 OutputBufferLength) \
    ((__NtDeviceIoControlFile)(char*)stubs[ID_NtDeviceIoControlFile])( \
        FileHandle, \
        Event, \
        ApcRoutine, \
        ApcContext, \
        IoStatusBlock, \
        IoControlCode, \
        InputBuffer, \
        InputBufferLength, \
        OutputBuffer, \
        OutputBufferLength)

#define SC_NtWaitForSingleObject(ObjectHandle, Alertable, TimeOut) \
    ((__NtWaitForSingleObject)(char*)stubs[ID_NtWaitForSingleObject])( \
        ObjectHandle, \
        Alertable, \
        TimeOut)

#define SC_NtClose(Handle) ((__NtClose)(char*)stubs[ID_NtClose])(Handle)

struct WrapFuncs {
    EXIT exit;
    WRAPNTCREATEUSERPROCESS WrapNtCreateUserProcess;
    WRAPSLEEP WrapSleep;
    WRAPRECV WrapRecv;
};
