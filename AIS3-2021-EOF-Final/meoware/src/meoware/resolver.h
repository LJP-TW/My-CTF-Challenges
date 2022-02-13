#pragma once

void* GetImageBase(WCHAR* target_dllname);

void* GetProcAddr(BYTE* imagebase, CHAR* procname);
