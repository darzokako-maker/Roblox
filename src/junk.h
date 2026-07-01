#pragma once
#include <windows.h>
#include <random>
#include <string>

class JunkCode {
private:
    static int GetRandom() {
        static int seed = GetTickCount() + (int)GetCurrentProcessId();
        return (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
public:
    // Gereksiz değişkenler
    static void AddJunkVariables() {
        volatile int junk1 = GetRandom() % 9999;
        volatile int junk2 = GetRandom() % 8888;
        volatile int junk3 = GetRandom() % 7777;
        volatile int junk4 = GetRandom() % 6666;
        
        junk1 = junk1 + junk2;
        junk2 = junk3 - junk1;
        junk3 = junk4 * junk2;
        junk4 = junk1 / (junk3 + 1);
        
        if(junk1 == -99999) MessageBoxA(0, "A", "B", 0);
        if(junk2 == -88888) Sleep(0);
        if(junk3 == -77777) Beep(0, 0);
        if(junk4 == -66666) GetTickCount();
    }
    
    // Sahte fonksiyon çağrıları
    static void FakeFunctionCalls() {
        GetTickCount();
        GetCurrentProcessId();
        GetCurrentThreadId();
        QueryPerformanceCounter(0);
        GetSystemTime(0);
        IsDebuggerPresent();
        GetVersionExA(0);
        GlobalMemoryStatusEx(0);
        GetKeyboardState(0);
        GetCursorPos(0);
        GetCaretBlinkTime();
        GetSystemMetrics(SM_CXSCREEN);
        GetSystemMetrics(SM_CYSCREEN);
        GetSysColor(COLOR_WINDOW);
        GetSysColorBrush(COLOR_BTNFACE);
        EnumFontsA(0, 0, 0, 0);
        GetACP();
        GetOEMCP();
        GetUserNameA(0, 0);
        GetComputerNameA(0, 0);
        GetWindowsDirectoryA(0, 0);
        GetSystemDirectoryA(0, 0);
        GetTempPathA(0, 0);
        GetCurrentDirectoryA(0, 0);
    }
    
    // Ölü kod blokları
    static void DeadCodeBlocks() {
        if(GetRandom() % 1000 == -1) {
            MessageBoxA(0, "X1", "Y1", 0);
            Sleep(999999);
            ExitProcess(0);
        }
        
        if(GetRandom() % 2000 == -1) {
            HANDLE h = CreateFileA("NUL", 0, 0, 0, 0, 0, 0);
            CloseHandle(h);
        }
        
        int x = GetRandom() % 100;
        for(int i = 0; i < 10; i++) {
            x = (x * 2 + i) % 1000;
            if(x == -9999) break;
        }
        
        volatile char buffer[256];
        for(int i = 0; i < 256; i++) {
            buffer[i] = (char)(GetRandom() % 256);
        }
    }
    
    // Sahte string işlemleri
    static void FakeStrings() {
        char str1[64], str2[64], str3[64];
        
        for(int i = 0; i < 64; i++) {
            str1[i] = 'A' + (GetRandom() % 26);
            str2[i] = 'a' + (GetRandom() % 26);
            str3[i] = '0' + (GetRandom() % 10);
        }
        
        lstrcpynA(str1, str2, 64);
        lstrcatA(str1, str3);
        lstrlenA(str1);
        CharUpperA(str1);
        CharLowerA(str2);
        
        wsprintfA(str1, "%d-%d-%d", GetRandom(), GetRandom(), GetRandom());
    }
    
    // Sahte matematik işlemleri
    static void FakeMath() {
        volatile double a = (double)GetRandom() / 1000.0;
        volatile double b = (double)GetRandom() / 2000.0;
        volatile double c = (double)GetRandom() / 3000.0;
        
        a = sin(a) * cos(b);
        b = tan(c) * sqrt(a + b);
        c = pow(a, b) + log(c + 1);
        
        volatile int i1 = (int)(a * 1000);
        volatile int i2 = (int)(b * 2000);
        volatile int i3 = (int)(c * 3000);
        
        i1 = i1 ^ i2;
        i2 = i2 | i3;
        i3 = i1 & i2;
    }
    
    // Ana junk fonksiyonu
    static void Generate() {
        AddJunkVariables();
        FakeFunctionCalls();
        DeadCodeBlocks();
        FakeStrings();
        FakeMath();
    }
};
