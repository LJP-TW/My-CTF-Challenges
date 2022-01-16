#include <Windows.h>

#include <iostream>

using namespace std;

#define PAGE_SIZE 0x1000

// - Manual Parse Symbol
// - String Obfuscator
// - Hook WriteFile to call wannaSleep 
// - Anti-analyze by rewrite ret address

typedef void* (*__LoadLibraryA)(LPCSTR lpLibFileName);
__LoadLibraryA _LoadLibraryA;

typedef BOOL(*__PathFileExistsA)(LPCSTR pszPath);
__PathFileExistsA _PathFileExistsA;

typedef void* (*__HeapCreate)(DWORD flOptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize);
__HeapCreate _HeapCreate;

typedef void* (*__CreateFileA)(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile);
__CreateFileA _CreateFileA;

typedef BOOL(*__GetFileSizeEx)(
	HANDLE hFile,
	PLARGE_INTEGER lpFileSize);
__GetFileSizeEx _GetFileSizeEx;

// HeapAlloc
typedef void* (*__RtlAllocateHeap)(
	HANDLE hHeap,
	DWORD dwFlags,
	SIZE_T dwBytes);
__RtlAllocateHeap _RtlAllocateHeap;

typedef BOOL(*__ReadFile)(
	HANDLE hFile,
	LPVOID lpBuffer,
	DWORD nNumberOfBytesToRead,
	LPDWORD lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped);
__ReadFile _ReadFile;

typedef BOOL(*__WriteFile)(
	HANDLE hFile,
	LPVOID lpBuffer,
	DWORD nNumberOfBytesToWrite,
	LPDWORD lpNumberOfBytesWritten,
	LPOVERLAPPED lpOverlapped);
__WriteFile _WriteFile;

typedef BOOL(*__CloseHandle)(HANDLE hObject);
__CloseHandle _CloseHandle;

class MT19937 {
public:
	MT19937();

	DWORD rand();

private:
	void twist();
	void twist_transfer();
	void nothing();
	DWORD mt[624];
	DWORD mti;
};

MT19937::MT19937()
{
	// 0613 is seed of wannaSleep
	mt[0] = 0613;
	mti = 0;

	for (int i = 1; i < 624; ++i) {
		mt[i] = 1812433253 * (mt[i - 1] ^ (mt[i - 1] >> 30)) + i;
	}
}

DWORD MT19937::rand()
{
	if (mti == 0) {
		nothing();
	}

	DWORD y = mt[mti];

	y = y ^ (y >> 11);
	y = y ^ ((y << 7) & 2636928640);
	y = y ^ ((y << 15) & 4022730752);
	y = y ^ (y >> 18);

	mti = (mti + 1) % 624;

	return y;
}

void MT19937::twist()
{
	nothing();

	for (int i = 0; i < 624; ++i) {
		DWORD y = (mt[i] & 0x80000000) + (mt[(i + 1) % 624] & 0x7fffffff);

		mt[i] = (y >> 1) ^ mt[(i + 397) % 624];

		if (y % 2 != 0) {
			mt[i] = mt[i] ^ 0x9908b0df;
		}
	}
}

void MT19937::twist_transfer()
{
	DWORD64 a[] = { 0, 0, 0 };
	a[0] = 1;
	a[1] = 2;
	a[2] = 0xdeadbeefdeadbeef;
}

void MT19937::nothing()
{
	int i = 0;

	twist_transfer();
}

void Obfuscator(CHAR* str)
{
	for (int i = 0; i < strlen(str); ++i) {
		str[i] = str[i] ^ 0x87 ^ i;
	}
}

void Obfuscator(WCHAR* str)
{
	for (int i = 0; i < wcslen(str); ++i) {
		str[i] = str[i] ^ 0xe887 ^ i;
	}
}

void* GetImageBase(WCHAR* target_dllname)
{
	BYTE* peb = (BYTE*)__readgsqword(0x60);
	BYTE* ldr = *(BYTE **)(peb + 0x18);
	BYTE* ldr_flink  = *(BYTE**)(ldr + 0x10);
	BYTE* data_flink = *(BYTE**)(ldr_flink);

	while (ldr_flink != data_flink) {
		WCHAR* dllname = *(WCHAR **)(data_flink + 0x60);

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
	DWORD num_funcs  = *(DWORD*)(export_dir + 0x14);
	BYTE* addr_funcs = imagebase + *(DWORD*)(export_dir + 0x1c);
	BYTE* addr_names = imagebase + *(DWORD*)(export_dir + 0x20);
	BYTE* addr_ordis = imagebase + *(DWORD*)(export_dir + 0x24);
	DWORD i;
	BYTE* addr_proc;
	DWORD ordi;

	for (i = 0; i < num_funcs; ++i) {
		CHAR* func_name = (CHAR*)(imagebase + *(DWORD*)(addr_names + i * 4));

		if (!_stricmp(func_name, procname)) {
			break;
		}
	}

	if (i >= num_funcs)
		return NULL;

	ordi = *(WORD*)(addr_ordis + i * 2);

	addr_proc = imagebase + *(DWORD*)(addr_funcs + ordi * 4);

	return addr_proc;
}

void wannaSleep(BYTE* content, DWORD size)
{
	MT19937 prng;
	DWORD64 i;

	for (i = 0; i < size; ++i) {
		content[i] ^= prng.rand() ;
	}
}

CHAR waNtdll[] = { 233, 0xe8, 242, 0xe8, 225, 0xe8, 232, 0xe8, 239, 0xe8, 172, 0xe8, 229, 0xe8, 236, 0xe8, 227, 0xe8, 0 };
CHAR caRtlAllocateHeap[] = { 213, 242, 233, 197, 239, 238, 238, 227, 238, 250, 232, 196, 238, 235, 249, 0 };
CHAR waKernel32[] = { 236, 232, 227, 232, 247, 232, 234, 232, 230, 232, 238, 232, 178, 232, 178, 232, 161, 232, 234, 232, 225, 232, 224, 232, 0 };
CHAR caLoadLibraryA[] = { 203, 233, 228, 224, 207, 235, 227, 242, 238, 252, 244, 205, 0 };
CHAR caHeapCreate[] = { 207, 227, 228, 244, 192, 240, 228, 225, 251, 235, 0 };
CHAR caCreateFileA[] = { 196, 244, 224, 229, 247, 231, 199, 233, 227, 235, 204, 0 };
CHAR caGetFileSizeEx[] = { 192, 227, 241, 194, 234, 238, 228, 211, 230, 244, 232, 201, 243, 0 };
CHAR caReadFile[] = { 213, 227, 228, 224, 197, 235, 237, 229, 0 };
CHAR caWriteFile[] = { 208, 244, 236, 240, 230, 196, 232, 236, 234, 0 };
CHAR caCloseHandle[] = { 196, 234, 234, 247, 230, 202, 224, 238, 235, 226, 232, 0 };
CHAR waShlwapi[] = { 244, 232, 238, 232, 233, 232, 243, 232, 226, 232, 242, 232, 232, 232, 174, 232, 235, 232, 226, 232, 225, 232, 0 };
CHAR caShlwapi[] = { 244, 238, 233, 243, 226, 242, 232, 174, 235, 226, 225, 0 };
CHAR caPathFileExistsA[] = { 215, 231, 241, 236, 197, 235, 237, 229, 202, 246, 228, 255, 255, 249, 200, 0 };

HANDLE ghFile;

void deobfstr()
{
	Obfuscator((WCHAR*)waNtdll);
	Obfuscator(caRtlAllocateHeap);
	Obfuscator((WCHAR*)waKernel32);
	Obfuscator(caLoadLibraryA);
	Obfuscator(caHeapCreate);
	Obfuscator(caCreateFileA);
	Obfuscator(caGetFileSizeEx);
	Obfuscator(caReadFile);
	Obfuscator(caWriteFile);
	Obfuscator(caCloseHandle);
	Obfuscator((WCHAR*)waShlwapi);
	Obfuscator(caShlwapi);
	Obfuscator(caPathFileExistsA);
}

void getprocs()
{
	BYTE* ntdll_base = (BYTE*)GetImageBase((WCHAR*)waNtdll);
	_RtlAllocateHeap = (__RtlAllocateHeap)GetProcAddr(ntdll_base, (CHAR*)caRtlAllocateHeap);

	BYTE* kernel32_base = (BYTE*)GetImageBase((WCHAR*)waKernel32);
	_LoadLibraryA = (__LoadLibraryA)GetProcAddr(kernel32_base, (CHAR*)caLoadLibraryA);
	_HeapCreate = (__HeapCreate)GetProcAddr(kernel32_base, (CHAR*)caHeapCreate);
	_CreateFileA = (__CreateFileA)GetProcAddr(kernel32_base, (CHAR*)caCreateFileA);
	_GetFileSizeEx = (__GetFileSizeEx)GetProcAddr(kernel32_base, (CHAR*)caGetFileSizeEx);
	_ReadFile = (__ReadFile)GetProcAddr(kernel32_base, (CHAR*)caReadFile);
	_WriteFile = (__WriteFile)GetProcAddr(kernel32_base, (CHAR*)caWriteFile);
	_CloseHandle = (__CloseHandle)GetProcAddr(kernel32_base, (CHAR*)caCloseHandle);

	_LoadLibraryA(caShlwapi);
	BYTE* shlwapi_base = (BYTE*)GetImageBase((WCHAR*)waShlwapi);
	_PathFileExistsA = (__PathFileExistsA)GetProcAddr(shlwapi_base, (CHAR*)caPathFileExistsA);
}

void WINAPI f5(void* p1, void* p2, void* p3, void* p4, void* p5)
{
}

void WINAPI preWriteFile(
	HANDLE hFile,
	LPVOID lpBuffer,
	DWORD nNumberOfBytesToWrite,
	LPDWORD lpNumberOfBytesWritten,
	LPOVERLAPPED lpOverlapped)
{
	if (hFile == ghFile) {
		wannaSleep((BYTE*)lpBuffer, nNumberOfBytesToWrite);
	}

	// Reset registers
	f5(hFile, lpBuffer, (void *)nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);

	// offset of ret: 0x5d
}

void hookWriteFile()
{
	CHAR trampoline[] = {
		0xff, 0x25, 0x10, 0x00, 0x00, 0x00, // jmp qword ptr [$+0x16]
		0xff, 0x25, 0x00, 0x00, 0x00, 0x00,	// jmp qword ptr [&WriteFile]
	};

	CHAR jmp2[] = {
		0xff, 0x25, 0x00, 0x00, 0x00, 0x00,             // jmp qword ptr [$+6]
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // $+6
	};

	// Origin WriteFile:
	// FF25 E2D10500: jmp qword ptr ds:[0x00007FFE366C21B8]

	DWORD offsetWriteFile;
	DWORD oldProtection;
	DWORD retoffset = 0x5d;

	offsetWriteFile = *(DWORD*)((BYTE*)_WriteFile + 2);
	offsetWriteFile -= (sizeof(trampoline) - 6);

	*(DWORD*)((BYTE*)trampoline + sizeof(trampoline) - 4) = offsetWriteFile;

	*(DWORD64*)((BYTE*)jmp2 + sizeof(jmp2) - 8) = (DWORD64)_WriteFile + sizeof(trampoline) - 6;

	// Patch WriteFile to trampoline
	VirtualProtect(_WriteFile, 0x2f, PAGE_EXECUTE_READWRITE, &oldProtection);

	for (int i = 0; i < sizeof(trampoline); ++i) {
		*((BYTE*)(_WriteFile) + i) = trampoline[i];
	}

	*(DWORD64*)((BYTE*)(_WriteFile)+0x16) = (DWORD64)preWriteFile;

	VirtualProtect(_WriteFile, 0x2f, PAGE_EXECUTE_READ, &oldProtection);

	// Patch ret to jmp
	VirtualProtect(preWriteFile, 0x100, PAGE_EXECUTE_READWRITE, &oldProtection);

	for (int i = 0; i < sizeof(jmp2); ++i) {
		*((BYTE*)(preWriteFile) + retoffset + i) = jmp2[i];
	}

	VirtualProtect(preWriteFile, 0x100, PAGE_EXECUTE_READ, &oldProtection);
}

void patchTwist()
{
	// Anti-analyze by rewrite ret address
	BYTE patch[] = {
		0x48, 0x6b, 0xc0, 0x05,                                     // imul rax, 0x5
		0x48, 0xb9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rcx, addr
	};
	BYTE* real_twist = (BYTE*)patchTwist;
	BYTE* twist_imul = (BYTE*)patchTwist;
	DWORD oldProtection;

	real_twist -= 0x96d;

	twist_imul -= 0x890;
	twist_imul += 0x4a;

	*(DWORD64 *)((BYTE*)patch + sizeof(patch) - 8) = (DWORD64)real_twist;

	VirtualProtect(twist_imul, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtection);

	for (int i = 0; i < sizeof(patch); ++i) {
		*((BYTE*)twist_imul + i) = patch[i];
	}

	VirtualProtect(twist_imul, sizeof(patch), PAGE_EXECUTE_READ, &oldProtection);
}

class deobfstrClass {
public:
	deobfstrClass() {
		deobfstr();
	};
};

class getprocsClass {
public:
	getprocsClass() {
		getprocs();
	};
};

class hookWriteFileClass {
public:
	hookWriteFileClass() {
		hookWriteFile();
	};
};

class patchTwistClass {
public:
	patchTwistClass() {
		patchTwist();
	};
};

deobfstrClass runDeobfstr;
getprocsClass runGetprocs;
hookWriteFileClass runHookWriteFile;
patchTwistClass runPatchTwist;

int main(int argc, char* argv[])
{
	if (argc != 2) {
		return 0;
	}

	if (_PathFileExistsA(argv[1])) {
		LARGE_INTEGER liSize;
		DWORD64 qwSize;
		HANDLE hHeap;
		BYTE* pContent;
		DWORD dwRead;
		BYTE* pFilename;
		BYTE baEnc[] = { '.', 'e', 'n', 'c' };
		int i;

		hHeap = _HeapCreate(HEAP_NO_SERIALIZE, PAGE_SIZE * 10, PAGE_SIZE * 100);

		ghFile = _CreateFileA(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		
		_GetFileSizeEx(ghFile, &liSize);
		qwSize = liSize.QuadPart;
		
		pContent = (BYTE*)_RtlAllocateHeap(hHeap, 0, qwSize);

		_ReadFile(ghFile, pContent, qwSize, &dwRead, NULL);

		_CloseHandle(ghFile);

		pFilename = (BYTE*)_RtlAllocateHeap(hHeap, 0, strlen(argv[1]) + 0x10);

		for (i = 0; i < strlen(argv[1]); ++i) {
			pFilename[i] = argv[1][i];
		}

		for (int j = 0; j < sizeof(baEnc); ++j, ++i) {
			pFilename[i] = baEnc[j];
		}

		pFilename[i] = 0;

		ghFile = _CreateFileA((LPCSTR)pFilename, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

		_WriteFile(ghFile, pContent, qwSize, NULL, NULL);
	}
}
