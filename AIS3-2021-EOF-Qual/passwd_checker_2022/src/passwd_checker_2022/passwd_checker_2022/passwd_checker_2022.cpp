#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <Wincrypt.h>

#include <iostream>
#include <cstdlib>

using namespace std;

#define IDC_EDIT_FLAG			152
#define IDC_BUTTON_OK			150

#ifdef __cplusplus
#define INITIALIZER(f) \
        static void f(void); \
        struct f##_t_ { f##_t_(void) { f(); } }; static f##_t_ f##_; \
        static void f(void)
#elif defined(_MSC_VER)
#pragma section(".CRT$XCU",read)
#define INITIALIZER2_(f,p) \
        static void f(void); \
        __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
        __pragma(comment(linker,"/include:" p #f "_")) \
        static void f(void)
#ifdef _WIN64
#define INITIALIZER(f) INITIALIZER2_(f,"")
#else
#define INITIALIZER(f) INITIALIZER2_(f,"_")
#endif
#else
#define INITIALIZER(f) \
        static void f(void) __attribute__((constructor)); \
        static void f(void)
#endif

void NTAPI MY_TLS_CALLBACK(PVOID DllHandle, DWORD Reason, PVOID Reserved);

#ifdef _M_AMD64
#pragma comment (linker, "/INCLUDE:_tls_used")
#pragma comment (linker, "/INCLUDE:p_tls_callback1")
#pragma const_seg(".CRT$XLF")
EXTERN_C const PIMAGE_TLS_CALLBACK p_tls_callback1[] = { MY_TLS_CALLBACK, 0 };
#pragma const_seg()
#endif

#ifdef _M_IX86
#pragma comment (linker, "/INCLUDE:__tls_used")
#pragma comment (linker, "/INCLUDE:_p_tls_callback1")
#pragma data_seg(".CRT$XLF")
EXTERN_C PIMAGE_TLS_CALLBACK p_tls_callback1[] = { MY_TLS_CALLBACK, 0 };
#pragma data_seg()
#endif

void NTAPI MY_TLS_CALLBACK(PVOID DllHandle, DWORD Reason, PVOID Reserved)
{
    if (Reason != DLL_PROCESS_ATTACH)
        return;

    // Patch .rdata
    char* this_mod = (char*)GetModuleHandleA(NULL);
    char* nt_hdr = this_mod + *(DWORD*)(this_mod + 0x3c);
    WORD optional_hdr_size = *(WORD*)(nt_hdr + 0x14);
    WORD sections_count = *(WORD*)(nt_hdr + 6);
    char* optional_hdr = nt_hdr + 0x18;
    char* section_hdr = optional_hdr + optional_hdr_size;
    char* rdata_section_hdr = NULL;

    char* p = section_hdr;
    for (int i = 0; i < sections_count; ++i) {
        if (!strcmp(p, ".rdata")) {
            rdata_section_hdr = p;
            break;
        }
        p += 0x28;
    }

    char* rdata = this_mod + *(DWORD*)(rdata_section_hdr + 0xc);
    DWORD rdata_size = *(DWORD*)(rdata_section_hdr + 0x8);

    // Scan
    for (int offset = 0; offset < rdata_size;) {
        int len = strlen(rdata + offset);

        if (len != 44) {
            if (len == 0) {
                offset += 1;
            }
            else {
                offset += len;
            }

            continue;
        }

        if (*(rdata + offset) == 'E' && *(rdata + offset + 43) == 'N') {
            // Patch it
            char* target = rdata + offset;
            DWORD rwx = 0x40;
            DWORD oldprotect;
            DWORD xor_key[] = { 2, 0, 12, 64, 0, 70, 91, 13, 7, 35, 20, 94, 7, 57, 85, 54, 2, 127, 3, 36, 25, 3, 13, 2, 23, 62, 0, 38, 8, 0, 0, 42, 0, 0, 0, 0, 29, 5, 24, 24, 67, 47, 63, 115 };
            
            VirtualProtect(target, 44, rwx, &oldprotect);

            for (int i = 0; i < 44; ++i) {
                *(target + i) ^= xor_key[i];
            }

            VirtualProtect(target, 44, oldprotect, &oldprotect);

            break;
        }
    }
}

BYTE encrypted[0x100];

// FLAG{NANI___Th1s_1s_flAg___OmG}
// "G2FtzpmZCkW9qA9an6Owmq5ggjunB5FluTeK+IuZ4yQ="
// NOTFLAG{THIS_IS_NOT_FLAG_______}
// "E2J4z66WDHCgvxlWlILStr8epTuHJ5FFuTeK6LmBwVnN"
BYTE cipherText[0x100];

bool Base64EncodeA(unsigned long* dlen, const unsigned char* src, unsigned long slen)
{
    if (src == NULL)
        return false;

    unsigned char* dst = cipherText;

    if (!CryptBinaryToStringA(src, slen, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, dlen))
        return false;

    SecureZeroMemory(cipherText, 0x100);

    if (!CryptBinaryToStringA(src, slen, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, (LPSTR)dst, dlen))
    {
        return false;
    }

    return true;
}

bool rc4_encrypt(char *plainText)
{
    HCRYPTPROV hCryptProv = 0;
    HCRYPTKEY key = 0;
    HCRYPTHASH hash;
    unsigned long cLen = 0;
    char password[] = "passwd_checker_2022_key_len_only_0x10_you_must_care_about_it_homie\0";
    unsigned long pLen = strlen(password);
    unsigned long len = 0;
    unsigned long enLen = 0;

    // Key Generation

    if (!CryptAcquireContextW(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0))
    {
        if (GetLastError() == NTE_BAD_KEYSET)
        {
            if (!CryptAcquireContextW(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET))
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    if (!CryptCreateHash(hCryptProv, CALG_SHA1, 0, 0, &hash))
    {
        return false;
    }

    if (!CryptHashData(hash, (const BYTE*)password, pLen, 0))
    {
        CryptDestroyHash(hash);
        return false;
    }

    if (!CryptDeriveKey(hCryptProv, CALG_RC4, hash, CRYPT_EXPORTABLE, &key))
    {
        CryptDestroyHash(hash);
        return false;
    }

    CryptDestroyHash(hash);

    // Encryption

    if (!CryptEncrypt(key, 0, TRUE, 0, NULL, &len, 0))
    {
        if (key) CryptDestroyKey(key);
        return false;
    }

    pLen = strlen(plainText);
    len = pLen + 1;
    enLen = len;
    SecureZeroMemory(encrypted, 0x100);
    memcpy_s(encrypted, len, plainText, pLen + 1);

    if (!CryptEncrypt(key, 0, TRUE, 0, encrypted, &len, enLen))
    {
        if (key) CryptDestroyKey(key);
        return false;
    }

    if (!Base64EncodeA(&cLen, encrypted, enLen))
    {
        if (key) CryptDestroyKey(key);
        return false;
    }

    if (!strcmp((char *)cipherText, "E2J4z66WDHCgvxlWlILStr8epTuHJ5FFuTeK6LmBwVnN")) {
        return true;
    }

    return false;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int main()
{
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Password Checker 2022",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style
        0,
        0,
        500,
        100,
        NULL,       // Parent window    
        NULL,       // Menu
        GetModuleHandleA(NULL),  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    // Create EditBox
    HWND hWndEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE, 
        TEXT("Edit"), 
        TEXT("NOTFLAG{DONATE_TO_ENABLE_OK_BUTTON}"),
        WS_CHILD | WS_VISIBLE, 
        10, 
        20, 
        380,
        20, 
        hwnd, 
        (HMENU)IDC_EDIT_FLAG,
        NULL, 
        NULL);

    // Create Button
    HWND hwndButton = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"OK",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        400,         // x position 
        20,         // y position 
        50,        // Button width
        20,        // Button height
        hwnd,     // Parent window
        (HMENU)IDC_BUTTON_OK,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.

    EnableWindow(hwndButton, false);
    ShowWindow(hwnd, 1);

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR buf1[0x101] = { '\0' };
    char buf2[0x101] = { '\0' };
    HWND hEditFlag;
    bool ok;

    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            
            EndPaint(hwnd, &ps);
        }
        return 0;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);
        
        switch (wmId)
        {
        case IDC_BUTTON_OK:
            hEditFlag = GetDlgItem(hwnd, IDC_EDIT_FLAG);
            GetWindowText(hEditFlag, buf1, 0x100);
            wcstombs(buf2, buf1, wcslen(buf1) + 1);
            ok = rc4_encrypt(buf2);
            if (ok) {
                MessageBoxW(hwnd, TEXT("Correct!"), buf1, MB_OK);
            }
            else {
                MessageBoxW(hwnd, TEXT("Failed..."), buf1, MB_OK);
            }
            return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}