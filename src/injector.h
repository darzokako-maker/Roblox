#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>

class Injector {
private:
    HANDLE hProcess;
    
    bool LoadLibraryInject(const std::string& dllPath) {
        size_t pathSize = dllPath.length() + 1;
        
        LPVOID pRemoteMemory = VirtualAllocEx(
            hProcess, NULL, pathSize,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
        );
        
        if(!pRemoteMemory) return false;
        
        if(!WriteProcessMemory(hProcess, pRemoteMemory, dllPath.c_str(), pathSize, NULL)) {
            VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
            return false;
        }
        
        LPTHREAD_START_ROUTINE pLoadLibrary = (LPTHREAD_START_ROUTINE)
            GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
        
        HANDLE hThread = CreateRemoteThread(
            hProcess, NULL, 0,
            pLoadLibrary, pRemoteMemory, 0, NULL
        );
        
        if(!hThread) {
            VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
            return false;
        }
        
        WaitForSingleObject(hThread, INFINITE);
        
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hThread);
        
        return true;
    }
    
    bool ManualMapInject(const std::string& dllPath) {
        std::ifstream file(dllPath, std::ios::binary | std::ios::ate);
        if(!file) return false;
        
        size_t size = file.tellg();
        std::vector<BYTE> buffer(size);
        file.seekg(0);
        file.read((char*)buffer.data(), size);
        file.close();
        
        IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)buffer.data();
        if(dos->e_magic != IMAGE_DOS_SIGNATURE) return false;
        
        IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(buffer.data() + dos->e_lfanew);
        if(nt->Signature != IMAGE_NT_SIGNATURE) return false;
        
        LPVOID remote = VirtualAllocEx(
            hProcess, NULL, nt->OptionalHeader.SizeOfImage,
            MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE
        );
        
        if(!remote) return false;
        
        WriteProcessMemory(hProcess, remote, buffer.data(),
            nt->OptionalHeader.SizeOfHeaders, NULL);
        
        IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
        for(int i = 0; i < nt->FileHeader.NumberOfSections; i++, sec++) {
            if(sec->SizeOfRawData) {
                WriteProcessMemory(hProcess,
                    (LPVOID)((uintptr_t)remote + sec->VirtualAddress),
                    buffer.data() + sec->PointerToRawData,
                    sec->SizeOfRawData, NULL);
            }
        }
        
        return true;
    }

public:
    Injector() : hProcess(NULL) {}
    
    bool Attach(DWORD pid) {
        hProcess = OpenProcess(
            PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
            PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
            FALSE, pid
        );
        return hProcess != NULL;
    }
    
    bool Inject(const std::string& path) {
        return ManualMapInject(path);
    }
    
    bool IsAttached() {
        return hProcess != NULL;
    }
    
    void Detach() {
        if(hProcess) {
            CloseHandle(hProcess);
            hProcess = NULL;
        }
    }
    
    ~Injector() { Detach(); }
};
