#pragma once
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <vector>

#pragma comment(lib, "psapi.lib")

class Memory {
public:
    static DWORD GetPID(const wchar_t* name) {
        DWORD pids[2048], needed;
        if(!EnumProcesses(pids, sizeof(pids), &needed)) return 0;
        
        DWORD count = needed / sizeof(DWORD);
        for(DWORD i = 0; i < count; i++) {
            if(pids[i] == 0) continue;
            
            HANDLE hProcess = OpenProcess(
                PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                FALSE, pids[i]
            );
            
            if(hProcess) {
                WCHAR processName[MAX_PATH];
                HMODULE hMod;
                DWORD cbNeeded;
                
                if(EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
                    GetModuleBaseNameW(hProcess, hMod, processName, MAX_PATH);
                    if(_wcsicmp(processName, name) == 0) {
                        CloseHandle(hProcess);
                        return pids[i];
                    }
                }
                CloseHandle(hProcess);
            }
        }
        return 0;
    }
    
    static uintptr_t GetModuleBase(DWORD pid, const wchar_t* moduleName) {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if(hSnapshot == INVALID_HANDLE_VALUE) return 0;
        
        MODULEENTRY32W me;
        me.dwSize = sizeof(MODULEENTRY32W);
        
        if(Module32FirstW(hSnapshot, &me)) {
            do {
                if(_wcsicmp(me.szModule, moduleName) == 0) {
                    CloseHandle(hSnapshot);
                    return (uintptr_t)me.modBaseAddr;
                }
            } while(Module32NextW(hSnapshot, &me));
        }
        
        CloseHandle(hSnapshot);
        return 0;
    }
};
