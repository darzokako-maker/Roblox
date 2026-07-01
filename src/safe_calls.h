#pragma once
#include <windows.h>

class SafeCalls {
public:
    // Güvenli MessageBox
    static int SafeMessageBox(HWND hWnd, const char* text, const char* title, UINT type) {
        typedef int (WINAPI *pMessageBoxA)(HWND, LPCSTR, LPCSTR, UINT);
        
        HMODULE hUser32 = GetModuleHandleA("user32.dll");
        if(!hUser32) return 0;
        
        pMessageBoxA func = (pMessageBoxA)GetProcAddress(hUser32, "MessageBoxA");
        if(!func) return 0;
        
        // API hooking kontrolü
        BYTE* ptr = (BYTE*)func;
        if(ptr[0] == 0xE9 || ptr[0] == 0xEB) return 0; // JMP tespiti
        
        return func(hWnd, text, title, type);
    }
    
    // Güvenli dosya açma
    static HANDLE SafeCreateFile(const char* path) {
        typedef HANDLE (WINAPI *pCreateFileA)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
        
        HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
        pCreateFileA func = (pCreateFileA)GetProcAddress(hKernel32, "CreateFileA");
        
        BYTE* ptr = (BYTE*)func;
        if(ptr[0] == 0xE9) return INVALID_HANDLE_VALUE;
        
        return func(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    
    // Güvenli bellek ayırma
    static LPVOID SafeVirtualAlloc(SIZE_T size) {
        typedef LPVOID (WINAPI *pVirtualAlloc)(LPVOID, SIZE_T, DWORD, DWORD);
        
        HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
        pVirtualAlloc func = (pVirtualAlloc)GetProcAddress(hKernel32, "VirtualAlloc");
        
        return func(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    }
    
    // Anti-hook kontrol
    static BOOL IsFunctionHooked(const char* module, const char* function) {
        HMODULE hMod = GetModuleHandleA(module);
        if(!hMod) return FALSE;
        
        BYTE* func = (BYTE*)GetProcAddress(hMod, function);
        if(!func) return FALSE;
        
        // JMP (0xE9), CALL (0xE8), PUSH/RET kontrolü
        if(func[0] == 0xE9 || func[0] == 0xE8) return TRUE;
        if(func[0] == 0x68 && func[5] == 0xC3) return TRUE;
        
        return FALSE;
    }
};
