#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <string>
#include <thread>
#include <fstream>
#include <random>
#include "junk.h"
#include "obfuscate.h"
#include "anti_analysis.h"
#include "safe_calls.h"

#pragma comment(lib, "comctl32.lib")

// Gereksiz global değişkenler (junk)
volatile int g_junk1 = 0xDEAD;
volatile int g_junk2 = 0xBEEF;
volatile int g_junk3 = 0xCAFE;
volatile int g_junk4 = 0xBABE;
volatile DWORD g_junk5 = 0x1337;
volatile DWORD g_junk6 = 0x6969;

// Sahte fonksiyonlar
void FakeFunc1() { volatile int x = 0; for(int i=0;i<100;i++) x+=i; }
void FakeFunc2() { volatile int x = 0; for(int i=0;i<200;i++) x*=i; }
void FakeFunc3() { volatile int x = 0; for(int i=0;i<300;i++) x^=i; }
void FakeFunc4() { volatile int x = 0; for(int i=0;i<400;i++) x-=i; }

Injector injector;
HWND hMainWnd, hScriptEdit, hLogBox, hStatusBar;
HWND hInjectBtn, hExecuteBtn, hClearBtn, hSaveBtn, hLoadBtn, hAutoInjectChk, hClearLogBtn;
HFONT hFont;
bool autoInject = false;
bool isInjected = false;

// Renkler
#define COLOR_BG        RGB(25, 25, 25)
#define COLOR_BG2       RGB(35, 35, 35)
#define COLOR_TEXT      RGB(220, 220, 220)
#define COLOR_GREEN     RGB(0, 200, 100)
#define COLOR_RED       RGB(255, 80, 80)
#define COLOR_BLUE      RGB(0, 170, 255)
#define COLOR_CYAN      RGB(0, 255, 255)

// Control ID'leri
#define ID_SCRIPTEDIT   101
#define ID_INJECTBTN    102
#define ID_EXECUTEBTN   103
#define ID_CLEARBTN     104
#define ID_SAVEBTN      105
#define ID_LOADBTN      106
#define ID_AUTOINJECT   107
#define ID_LOGBOX       108
#define ID_CLEARLOGBTN  109

// Durum güncelleme
void UpdateStatus(const char* text) {
    JunkCode::Generate();
    SetWindowTextA(hStatusBar, text);
    JunkCode::FakeMath();
}

// Log ekleme
void AddLog(const char* text, COLORREF color) {
    JunkCode::AddJunkVariables();
    Obfuscator::FakeControlFlow();
    
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    char timeStr[50];
    sprintf_s(timeStr, "[%02d:%02d:%02d] ", st.wHour, st.wMinute, st.wSecond);
    
    std::string fullText = std::string(timeStr) + text + "\r\n";
    
    int len = GetWindowTextLengthA(hLogBox);
    SendMessageA(hLogBox, EM_SETSEL, len, len);
    
    CHARFORMAT2A cf = {0};
    cf.cbSize = sizeof(CHARFORMAT2A);
    cf.dwMask = CFM_COLOR;
    cf.crTextColor = color;
    
    SendMessageA(hLogBox, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    SendMessageA(hLogBox, EM_REPLACESEL, FALSE, (LPARAM)fullText.c_str());
    
    JunkCode::DeadCodeBlocks();
}

// Roblox'a enjekte
void InjectToRoblox() {
    JunkCode::Generate();
    Obfuscator::ControlFlowFlatten();
    
    AddLog("Roblox araniyor...", COLOR_BLUE);
    
    DWORD pid = Memory::GetPID(L"RobloxPlayerBeta.exe");
    if(!pid) pid = Memory::GetPID(L"RobloxPlayerLauncher.exe");
    
    if(!pid) {
        AddLog("Roblox bulunamadi!", COLOR_RED);
        return;
    }
    
    char buf[100];
    sprintf_s(buf, "Roblox bulundu! PID: %d", pid);
    AddLog(buf, COLOR_GREEN);
    
    if(!injector.Attach(pid)) {
        AddLog("Process acilamadi!", COLOR_RED);
        return;
    }
    
    AddLog("DLL enjekte ediliyor...", COLOR_BLUE);
    
    char dllPath[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, dllPath);
    strcat_s(dllPath, "\\exploit.dll");
    
    if(injector.Inject(dllPath)) {
        AddLog("Basarili! Executor hazir.", COLOR_GREEN);
        isInjected = true;
        EnableWindow(hExecuteBtn, TRUE);
        EnableWindow(hInjectBtn, FALSE);
    } else {
        AddLog("Enjeksiyon basarisiz!", COLOR_RED);
    }
    
    FakeFunc1();
    JunkCode::FakeStrings();
}

// Script çalıştır
void ExecuteScript() {
    JunkCode::Generate();
    Obfuscator::FakeControlFlow();
    
    int len = GetWindowTextLengthA(hScriptEdit);
    if(len == 0 || !isInjected) return;
    
    char* script = new char[len + 1];
    GetWindowTextA(hScriptEdit, script, len + 1);
    
    AddLog("Script calistiriliyor...", COLOR_BLUE);
    
    HMODULE dll = GetModuleHandleA("exploit.dll");
    if(!dll) dll = LoadLibraryA("exploit.dll");
    
    if(dll) {
        typedef void (*Exec)(const char*);
        Exec exec = (Exec)GetProcAddress(dll, "ExecuteExploit");
        if(exec) {
            exec(script);
            AddLog("Script tamamlandi.", COLOR_GREEN);
        }
    }
    
    delete[] script;
    FakeFunc2();
    JunkCode::FakeMath();
}

// WndProc
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Her mesajda junk code
    volatile int j1 = GetTickCount() % 1000;
    volatile int j2 = j1 * 2;
    j1 = j2 - j1;
    
    switch(msg) {
        case WM_CREATE: {
            AntiAnalysis::Initialize();
            JunkCode::Generate();
            
            hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, FIXED_PITCH, "Consolas");
            
            // Başlık
            CreateWindowA("STATIC", Obfuscator::HideString("EXECUTOR"),
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                10, 5, 600, 25, hWnd, NULL, NULL, NULL);
            
            // Script editör
            hScriptEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL,
                10, 35, 600, 200, hWnd, (HMENU)ID_SCRIPTEDIT, NULL, NULL);
            SendMessage(hScriptEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Butonlar
            hInjectBtn = CreateWindowA("BUTTON", "ENJEKTE ET",
                WS_VISIBLE | WS_CHILD,
                10, 245, 110, 30, hWnd, (HMENU)ID_INJECTBTN, NULL, NULL);
            
            hExecuteBtn = CreateWindowA("BUTTON", "CALISTIR",
                WS_VISIBLE | WS_CHILD | WS_DISABLED,
                125, 245, 110, 30, hWnd, (HMENU)ID_EXECUTEBTN, NULL, NULL);
            
            hClearBtn = CreateWindowA("BUTTON", "TEMIZLE",
                WS_VISIBLE | WS_CHILD,
                240, 245, 80, 30, hWnd, (HMENU)ID_CLEARBTN, NULL, NULL);
            
            hSaveBtn = CreateWindowA("BUTTON", "KAYDET",
                WS_VISIBLE | WS_CHILD,
                325, 245, 80, 30, hWnd, (HMENU)ID_SAVEBTN, NULL, NULL);
            
            hLoadBtn = CreateWindowA("BUTTON", "YUKLE",
                WS_VISIBLE | WS_CHILD,
                410, 245, 80, 30, hWnd, (HMENU)ID_LOADBTN, NULL, NULL);
            
            hClearLogBtn = CreateWindowA("BUTTON", "LOG TEMIZLE",
                WS_VISIBLE | WS_CHILD,
                495, 245, 90, 30, hWnd, (HMENU)ID_CLEARLOGBTN, NULL, NULL);
            
            hAutoInjectChk = CreateWindowA("BUTTON", "AUTO-INJECT",
                WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                10, 280, 150, 20, hWnd, (HMENU)ID_AUTOINJECT, NULL, NULL);
            
            // Log kutusu
            LoadLibraryA("msftedit.dll");
            hLogBox = CreateWindowExA(WS_EX_CLIENTEDGE, RICHEDIT_CLASSA, "",
                WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
                10, 305, 600, 130, hWnd, (HMENU)ID_LOGBOX, NULL, NULL);
            SendMessage(hLogBox, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLogBox, EM_SETBKGNDCOLOR, 0, COLOR_BG);
            
            // Status bar
            hStatusBar = CreateWindowA("STATIC", "Hazir",
                WS_VISIBLE | WS_CHILD | SS_LEFT | SS_SUNKEN,
                0, 442, 620, 20, hWnd, NULL, NULL, NULL);
            
            AddLog("Executor baslatildi.", COLOR_GREEN);
            
            FakeFunc3();
            break;
        }
        
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT:
            SetBkColor((HDC)wParam, COLOR_BG);
            SetTextColor((HDC)wParam, COLOR_TEXT);
            return (LRESULT)CreateSolidBrush(COLOR_BG);
        
        case WM_ERASEBKGND: {
            RECT rect;
            GetClientRect(hWnd, &rect);
            HBRUSH brush = CreateSolidBrush(COLOR_BG);
            FillRect((HDC)wParam, &rect, brush);
            DeleteObject(brush);
            return 1;
        }
        
        case WM_COMMAND: {
            JunkCode::Generate();
            
            switch(LOWORD(wParam)) {
                case ID_INJECTBTN:
                    CreateThread(NULL, 0, [](LPVOID) -> DWORD {
                        JunkCode::Generate();
                        InjectToRoblox();
                        return 0;
                    }, NULL, 0, NULL);
                    break;
                    
                case ID_EXECUTEBTN:
                    CreateThread(NULL, 0, [](LPVOID) -> DWORD {
                        JunkCode::Generate();
                        ExecuteScript();
                        return 0;
                    }, NULL, 0, NULL);
                    break;
                    
                case ID_CLEARBTN:
                    SetWindowTextA(hScriptEdit, "");
                    break;
                    
                case ID_CLEARLOGBTN:
                    SetWindowTextA(hLogBox, "");
                    break;
                    
                case ID_AUTOINJECT:
                    autoInject = SendMessage(hAutoInjectChk, BM_GETCHECK, 0, 0) == BST_CHECKED;
                    break;
            }
            break;
        }
        
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    
    FakeFunc4();
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// Auto-inject thread
void AutoInjectLoop() {
    while(true) {
        JunkCode::Generate();
        if(autoInject && !isInjected) {
            DWORD pid = Memory::GetPID(L"RobloxPlayerBeta.exe");
            if(pid) InjectToRoblox();
        }
        Sleep(5000);
    }
}

// WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Junk code
    JunkCode::Generate();
    Obfuscator::ControlFlowFlatten();
    FakeFunc1();
    FakeFunc2();
    
    // Anti-analiz
    AntiAnalysis::Initialize();
    
    // Sahte işlemler
    JunkCode::FakeFunctionCalls();
    JunkCode::DeadCodeBlocks();
    JunkCode::FakeStrings();
    JunkCode::FakeMath();
    
    // Pencere sınıfı
    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(COLOR_BG);
    wc.lpszClassName = "WinClass";
    
    RegisterClassExA(&wc);
    
    // Pencere oluştur
    hMainWnd = CreateWindowExA(0, "WinClass", "Executor",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        (GetSystemMetrics(SM_CXSCREEN) - 630) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - 490) / 2,
        630, 490,
        NULL, NULL, hInstance, NULL);
    
    ShowWindow(hMainWnd, nCmdShow);
    
    // Auto-inject thread
    std::thread autoThread(AutoInjectLoop);
    autoThread.detach();
    
    // Mesaj döngüsü
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        JunkCode::Generate();
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return msg.wParam;
}
