#pragma once
#include <windows.h>
#include <winternl.h>

class AntiDebug {
public:
    static BOOL IsDebug() {
        BOOL debugger = FALSE;
        CheckRemoteDebuggerPresent(GetCurrentProcess(), &debugger);
        if(debugger) return TRUE;
        
        if(IsDebuggerPresent()) return TRUE;
        
        __try {
            __asm {
                int 3
                xor eax, eax
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            return FALSE;
        }
        return TRUE;
    }
    
    static void Initialize() {
        CreateThread(NULL, 0, [](LPVOID) -> DWORD {
            while(true) {
                if(IsDebug()) ExitProcess(0);
                
                CONTEXT ctx = {};
                ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
                GetThreadContext(GetCurrentThread(), &ctx);
                if(ctx.Dr0 || ctx.Dr1 || ctx.Dr2 || ctx.Dr3)
                    ExitProcess(0);
                    
                Sleep(100);
            }
            return 0;
        }, NULL, 0, NULL);
    }
};
