#include <Windows.h>

#include "resolver.h"

void* GetImageBase(WCHAR* target_dllname)
{
    BYTE* peb = (BYTE*)__readgsqword(0x60);
    BYTE* ldr = *(BYTE**)(peb + 0x18);
    BYTE* ldr_flink = *(BYTE**)(ldr + 0x10);
    BYTE* data_flink = *(BYTE**)(ldr_flink);

    while (ldr_flink != data_flink) {
        WCHAR* dllname = *(WCHAR**)(data_flink + 0x60);

        if (dllname) {
            if (!_wcsicmp(target_dllname, dllname)) {
                return *(void**)(data_flink + 0x30);
            }
        }

        data_flink = *(BYTE**)(data_flink);
    }

    return NULL;
}

void* GetProcAddr(BYTE* imagebase, CHAR* procname)
{
    BYTE* pe_hdr = imagebase + *(DWORD*)(imagebase + 0x3c);
    BYTE* export_dir = imagebase + *(DWORD*)(pe_hdr + 0x88);
    DWORD export_dir_sz = *(DWORD*)(pe_hdr + 0x8c);
    DWORD num_funcs = *(DWORD*)(export_dir + 0x14);
    DWORD* addr_funcs = (DWORD*)(imagebase + *(DWORD*)(export_dir + 0x1c));
    DWORD* addr_names = (DWORD*)(imagebase + *(DWORD*)(export_dir + 0x20));
    WORD* addr_ordis = (WORD*)(imagebase + *(DWORD*)(export_dir + 0x24));
    DWORD i;
    BYTE* addr;
    DWORD ordi;

    for (i = 0; i < num_funcs; ++i) {
        CHAR* func_name = (CHAR*)(imagebase + addr_names[i]);

        if (!_stricmp(func_name, procname)) {
            break;
        }
    }

    if (i >= num_funcs)
        return NULL;

    ordi = addr_ordis[i];

    addr = imagebase + addr_funcs[ordi];

    // Forwarder?
    if (export_dir < addr && addr < export_dir + export_dir_sz) {
        // Yes it is
        WCHAR wforwarder_dll[260];
        CHAR  forwarder_dll[260];
        CHAR dll[] = ".dll";
        BYTE* forwarder_procname;
        BYTE* forwarder_dll_imagebase;
        DWORD i = 0;

        forwarder_procname = addr;

        while (*forwarder_procname++ != '.');

        while (addr + i < forwarder_procname - 1) {
            wforwarder_dll[i] = addr[i];
            forwarder_dll[i] = addr[i];
            i++;
        }

        for (DWORD j = 0; j < sizeof(dll); ++j) {
            wforwarder_dll[i+j] = dll[j];
            forwarder_dll[i+j] = dll[j];
        }

        forwarder_dll_imagebase = (BYTE*)GetImageBase(wforwarder_dll);
        if (!forwarder_dll_imagebase) {
            forwarder_dll_imagebase = (BYTE*)LoadLibraryA(forwarder_dll);
        }
        addr = (BYTE*)GetProcAddr(forwarder_dll_imagebase, (CHAR*)forwarder_procname);
    }

    return addr;
}