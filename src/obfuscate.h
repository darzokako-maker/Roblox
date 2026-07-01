#pragma once
#include <windows.h>

class Obfuscator {
public:
    // String gizleme
    static char* HideString(const char* str) {
        static char buffer[512];
        char key = 'X' ^ 'K' ^ 0x69;
        int len = lstrlenA(str);
        
        for(int i = 0; i < len; i++) {
            buffer[i] = str[i] ^ key ^ (i % 255);
        }
        buffer[len] = 0;
        
        return buffer;
    }
    
    // Fonksiyon adresini gizle
    static FARPROC GetHiddenProc(HMODULE hModule, const char* procName) {
        char hidden[256];
        char key = 'H' ^ 'I' ^ 'D' ^ 0x42;
        
        lstrcpynA(hidden, procName, 256);
        for(int i = 0; i < lstrlenA(hidden); i++) {
            hidden[i] ^= key;
        }
        
        FARPROC proc = GetProcAddress(hModule, procName);
        
        for(int i = 0; i < lstrlenA(hidden); i++) {
            hidden[i] ^= key;
        }
        
        return proc;
    }
    
    // Kontrol akışı düzleştirme
    static void ControlFlowFlatten() {
        volatile int state = GetTickCount() % 5;
        
        switch(state) {
            case 0: state = 1; break;
            case 1: state = 2; break;
            case 2: state = 3; break;
            case 3: state = 4; break;
            case 4: state = 0; break;
            default: state = 0; break;
        }
    }
    
    // Sahte kontrol akışı
    static void FakeControlFlow() {
        volatile int x = GetTickCount() % 100;
        volatile int y = 0;
        
        if(x > 50) {
            y = 1;
            if(x < 25) y = 2;
        } else {
            y = 3;
            if(x > 75) y = 4;
        }
        
        switch(y) {
            case 1: x++; break;
            case 2: x--; break;
            case 3: x*=2; break;
            case 4: x/=2; break;
        }
    }
};
