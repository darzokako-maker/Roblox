#pragma once
#include <windows.h>
#include <winternl.h>
#include <psapi.h>
#include <intrin.h>

class AntiAnalysis {
private:
    // Sandbox tespiti
    static BOOL IsSandbox() {
        // Düşük RAM kontrolü
        MEMORYSTATUSEX mem;
        mem.dwLength = sizeof(mem);
        GlobalMemoryStatusEx(&mem);
        if(mem.ullTotalPhys < 2147483648) return TRUE; // 2GB'dan az
        
        // Düşük CPU kontrolü
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        if(si.dwNumberOfProcessors < 2) return TRUE;
        
        // Küçük disk kontrolü
        ULARGE_INTEGER totalBytes;
        GetDiskFreeSpaceExA("C:\\", NULL, &totalBytes, NULL);
        if(totalBytes.QuadPart < 53687091200) return TRUE; // 50GB'dan az
        
        return FALSE;
    }
    
    // VM tespiti
    static BOOL IsVirtualMachine() {
        // VMware
        __try {
            __asm {
                push edx
                push ecx
                push ebx
                mov eax, 'VMXh'
                mov ebx, 0
                mov ecx, 10
                mov edx, 'VX'
                in eax, dx
                cmp ebx, 'VMXh'
                je vm_detected
                mov eax, 0
                jmp done
                vm_detected:
                mov eax, 1
                done:
                pop ebx
                pop ecx
                pop edx
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            return FALSE;
        }
        
        // VirtualBox
        if(GetModuleHandleA("VBoxHook.dll")) return TRUE;
        if(GetModuleHandleA("vboxmrxnp.dll")) return TRUE;
        
        return FALSE;
    }
    
    // Debugger tespiti
    static BOOL IsBeingAnalyzed() {
        BOOL debug = FALSE;
        CheckRemoteDebuggerPresent(GetCurrentProcess(), &debug);
        if(debug) return TRUE;
        
        if(IsDebuggerPresent()) return TRUE;
        
        // NtGlobalFlag
        PDWORD pNtGlobalFlag = (PDWORD)(__readgsqword(0x60) + 0xBC);
        if(*pNtGlobalFlag & 0x70) return TRUE;
        
        // Heap flags
        DWORD* heapFlags = (DWORD*)(__readgsqword(0x60) + 0x104);
        if(*heapFlags & 2) return TRUE;
        
        return FALSE;
    }
    
public:
    static void Initialize() {
        if(IsSandbox() || IsVirtualMachine() || IsBeingAnalyzed()) {
            // Kendini gizle - crash yerine normal davran
            Sleep(5000);
        }
    }
    
    static void ProtectMemory() {
        // Sayfa koruması değiştir
        HMODULE hModule = GetModuleHandleA(NULL);
        MODULEINFO modInfo;
        GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo));
        
        DWORD oldProtect;
        VirtualProtect(modInfo.lpBaseOfDll, modInfo.SizeOfImage, PAGE_EXECUTE_READ, &oldProtect);
    }
};
