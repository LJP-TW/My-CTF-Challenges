#pragma once
#include <Windows.h>

typedef DWORD(*XCRC32)(BYTE* buf, DWORD len, DWORD init);
DWORD xcrc32(BYTE* buf, DWORD len, DWORD init);
