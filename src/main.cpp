#include <windows.h>
#include <commctrl.h>
#include <string>
#include <thread>
#include <fstream>
#include <shlobj.h>
#include "injector.h"
#include "memory.h"
#include "anti_debug.h"
#include "xorstr.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Global değişkenler
Injector injector;
HWND hMainWnd, hScriptEdit, hLogBox, hStatusBar;
HWND hInjectBtn, hExecuteBtn, hClearBtn, hSaveBtn, hLoadBtn, hAutoInjectChk, hClearLogBtn;
HFONT hFont;
bool autoInject = false;
bool isInjected = false;
COLORREF currentTextColor = RGB(220, 220, 220);

// Renkler
#define COLOR_BG        RGB(25, 25, 25)
#define COLOR_BG2       RGB(35, 35, 35)
#define COLOR_BG3       RGB(45, 45, 45)
#define COLOR_BORDER    RGB(60, 60, 60)
#define COLOR_TEXT      RGB(220, 220, 220)
#define COLOR_GREEN     RGB(0, 200, 100)
#define COLOR_RED       RGB(255, 80, 80)
#define COLOR_BLUE      RGB(0, 170, 255)
#define COLOR_PURPLE    RGB(150, 100, 255)
#define COLOR_ORANGE    RGB(255, 170, 0)
#define COLOR_YELLOW    RGB(255, 255, 0)
#define COLOR_CYAN      RGB(0, 255, 255)
#define COLOR_GRAY      RGB(150, 150, 150)

// Control ID'leri
#define ID_SCRIPTEDIT   101
#define ID_INJECTBTN    102
#define ID_EXECUTEBTN   103
#define ID_CLEARBTN     104
#define ID_SAVEBTN      105
#define ID_LOADBTN      106
#define ID_AUTOINJECT   107
#define ID_LOGBOX       108
#define ID_STATUSBAR    109
#define ID_CLEARLOGBTN  110

// Fonksiyon prototipleri
void AddLog(const char* text, COLORREF color = COLOR_TEXT);
void UpdateStatus(const char* text);
void InjectToRoblox();
void ExecuteScript();
void ClearScript();
void SaveScript();
void LoadScript();
void AutoInjectLoop();

// Status bar güncelleme
void UpdateStatus(const char* text) {
    SetWindowTextA(hStatusBar, text);
}

// Log ekleme fonksiyonu
void AddLog(const char* text, COLORREF color) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    char timeStr[50];
    sprintf_s(timeStr, "[%02d:%02d:%02d] ", st.wHour, st.wMinute, st.wSecond);
    
    std::string fullText = std::string(timeStr) + text + "\r\n";
    
    int len = GetWindowTextLengthA(hLogBox);
    SendMessageA(hLogBox, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    
    CHARFORMAT2A cf;
    memset(&cf, 0, sizeof(CHARFORMAT2A));
    cf.cbSize = sizeof(CHARFORMAT2A);
    cf.dwMask = CFM_COLOR | CFM_BOLD;
    cf.crTextColor = color;
    cf.dwEffects = 0;
    
    SendMessageA(hLogBox, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    SendMessageA(hLogBox, EM_REPLACESEL, FALSE, (LPARAM)fullText.c_str());
    
    // Scroll to bottom
    SendMessageA(hLogBox, EM_SCROLL, SB_BOTTOM, 0);
}

// Roblox'a enjekte et
void InjectToRoblox() {
    if(isInjected) {
        AddLog("Zaten enjekte edildi!", COLOR_YELLOW);
        return;
    }
    
    AddLog("Roblox araniyor...", COLOR_BLUE);
    UpdateStatus("Roblox araniyor...");
    
    DWORD pid = Memory::GetPID(L"RobloxPlayerBeta.exe");
    if(!pid) {
        pid = Memory::GetPID(L"RobloxPlayerLauncher.exe");
    }
    
    if(!pid) {
        AddLog("Roblox bulunamadi! Once Roblox'u acin.", COLOR_RED);
        UpdateStatus("Roblox bulunamadi");
        MessageBoxA(hMainWnd, "Roblox acik degil!\nLutfen once Roblox'u baslatin.", "Hata", MB_ICONERROR);
        return;
    }
    
    char pidText[100];
    sprintf_s(pidText, "Roblox bulundu! PID: %d", pid);
    AddLog(pidText, COLOR_GREEN);
    
    if(!injector.Attach(pid)) {
        AddLog("Process acilamadi! Yonetici olarak calistirin.", COLOR_RED);
        UpdateStatus("Yetki hatasi");
        MessageBoxA(hMainWnd, "Process acilamadi!\nYonetici olarak calistirmayi deneyin.", "Hata", MB_ICONERROR);
        return;
    }
    
    AddLog("DLL enjekte ediliyor...", COLOR_BLUE);
    UpdateStatus("DLL enjekte ediliyor...");
    
    char dllPath[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, dllPath);
    strcat_s(dllPath, "\\exploit.dll");
    
    if(injector.Inject(dllPath)) {
        AddLog("Basarili! Executor hazir.", COLOR_GREEN);
        UpdateStatus("Enjekte edildi - Hazir");
        isInjected = true;
        
        SetWindowTextA(hInjectBtn, "✓ Enjekte Edildi");
        EnableWindow(hInjectBtn, FALSE);
        EnableWindow(hExecuteBtn, TRUE);
        
        MessageBoxA(hMainWnd, "Basarili!\nExecutor hazir, script calistirabilirsiniz.", "Bilgi", MB_ICONINFORMATION);
    } else {
        AddLog("Enjeksiyon basarisiz!", COLOR_RED);
        UpdateStatus("Enjeksiyon hatasi");
        MessageBoxA(hMainWnd, "Enjeksiyon basarisiz!\nDLL dosyasini kontrol edin.", "Hata", MB_ICONERROR);
        injector.Detach();
    }
}

// Script çalıştır
void ExecuteScript() {
    if(!isInjected) {
        AddLog("Once Roblox'a enjekte edin!", COLOR_ORANGE);
        MessageBoxA(hMainWnd, "Once Roblox'a enjekte etmelisiniz!", "Uyari", MB_ICONWARNING);
        return;
    }
    
    int len = GetWindowTextLengthA(hScriptEdit);
    if(len == 0) {
        AddLog("Script kutusu bos!", COLOR_ORANGE);
        MessageBoxA(hMainWnd, "Script yazmayi unuttunuz!", "Uyari", MB_ICONWARNING);
        return;
    }
    
    char* script = new char[len + 1];
    GetWindowTextA(hScriptEdit, script, len + 1);
    
    AddLog("Script calistiriliyor...", COLOR_PURPLE);
    UpdateStatus("Script calistiriliyor...");
    
    HMODULE dll = GetModuleHandleA("exploit.dll");
    if(!dll) {
        dll = LoadLibraryA("exploit.dll");
    }
    
    if(!dll) {
        AddLog("exploit.dll bulunamadi!", COLOR_RED);
        delete[] script;
        return;
    }
    
    typedef void (*ExecuteFunc)(const char*);
    ExecuteFunc exec = (ExecuteFunc)GetProcAddress(dll, "ExecuteExploit");
    
    if(exec) {
        exec(script);
        AddLog("Script basariyla calistirildi.", COLOR_GREEN);
        UpdateStatus("Script calistirildi");
    } else {
        AddLog("ExecuteExploit fonksiyonu DLL icinde bulunamadi!", COLOR_RED);
        UpdateStatus("Fonksiyon bulunamadi");
    }
    
    delete[] script;
}

// Script temizle
void ClearScript() {
    SetWindowTextA(hScriptEdit, "");
    AddLog("Script temizlendi.", COLOR_GRAY);
    UpdateStatus("Script temizlendi");
}

// Script kaydet
void SaveScript() {
    int len = GetWindowTextLengthA(hScriptEdit);
    if(len == 0) {
        AddLog("Kaydedilecek script yok!", COLOR_ORANGE);
        return;
    }
    
    char* script = new char[len + 1];
    GetWindowTextA(hScriptEdit, script, len + 1);
    
    OPENFILENAMEA ofn = {0};
    char fileName[MAX_PATH] = "script.txt";
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hMainWnd;
    ofn.lpstrFilter = "Lua Scripts\0*.lua\0Text Files\0*.txt\0All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrDefExt = "lua";
    ofn.Flags = OFN_OVERWRITEPROMPT;
    
    if(GetSaveFileNameA(&ofn)) {
        std::ofstream file(fileName);
        if(file.is_open()) {
            file << script;
            file.close();
            
            char msg[200];
            sprintf_s(msg, "Script kaydedildi: %s", fileName);
            AddLog(msg, COLOR_GREEN);
            UpdateStatus("Script kaydedildi");
        } else {
            AddLog("Dosya kaydedilemedi!", COLOR_RED);
        }
    }
    
    delete[] script;
}

// Script yükle
void LoadScript() {
    OPENFILENAMEA ofn = {0};
    char fileName[MAX_PATH] = "";
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hMainWnd;
    ofn.lpstrFilter = "Lua Scripts\0*.lua\0Text Files\0*.txt\0All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    
    if(GetOpenFileNameA(&ofn)) {
        std::ifstream file(fileName);
        if(file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            SetWindowTextA(hScriptEdit, content.c_str());
            
            char msg[200];
            sprintf_s(msg, "Script yuklendi: %s", fileName);
            AddLog(msg, COLOR_GREEN);
            UpdateStatus("Script yuklendi");
        } else {
            AddLog("Dosya acilamadi!", COLOR_RED);
        }
    }
}

// Log temizle
void ClearLog() {
    SetWindowTextA(hLogBox, "");
    AddLog("Log temizlendi.", COLOR_GRAY);
}

// Auto-inject thread
void AutoInjectLoop() {
    while(true) {
        if(autoInject && !isInjected) {
            DWORD pid = Memory::GetPID(L"RobloxPlayerBeta.exe");
            if(!pid) {
                pid = Memory::GetPID(L"RobloxPlayerLauncher.exe");
            }
            if(pid) {
                InjectToRoblox();
            }
        }
        Sleep(3000);
    }
}

// WndProc - Ana mesaj işleyici
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            // Modern font oluştur
            hFont = CreateFontA(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            
            // Üst başlık paneli
            HWND hTitleBg = CreateWindowA("STATIC", "",
                WS_VISIBLE | WS_CHILD,
                0, 0, 650, 50, hWnd, NULL, NULL, NULL);
            
            CreateWindowA("STATIC", "EXECUTOR v3.0",
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                0, 5, 650, 25, hWnd, NULL, NULL, NULL);
            
            CreateWindowA("STATIC", "Advanced Script Executor",
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                0, 28, 650, 18, hWnd, NULL, NULL, NULL);
            
            // Script editör label
            CreateWindowA("STATIC", "SCRIPT EDITOR:",
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                10, 55, 200, 20, hWnd, NULL, NULL, NULL);
            
            // Script editör
            hScriptEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | 
                ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL,
                10, 75, 630, 220, hWnd, (HMENU)ID_SCRIPTEDIT, NULL, NULL);
            SendMessage(hScriptEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hScriptEdit, EM_SETBKGNDCOLOR, 0, COLOR_BG2);
            
            // Buton paneli arkaplan
            CreateWindowA("STATIC", "",
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                10, 305, 630, 70, hWnd, NULL, NULL, NULL);
            
            // Butonlar
            hInjectBtn = CreateWindowA("BUTTON", "ENJEKTE ET",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_CENTER,
                15, 312, 120, 35, hWnd, (HMENU)ID_INJECTBTN, NULL, NULL);
            SendMessage(hInjectBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hExecuteBtn = CreateWindowA("BUTTON", "CALISTIR",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_CENTER | WS_DISABLED,
                140, 312, 120, 35, hWnd, (HMENU)ID_EXECUTEBTN, NULL, NULL);
            SendMessage(hExecuteBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hClearBtn = CreateWindowA("BUTTON", "TEMIZLE",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_CENTER,
                265, 312, 90, 35, hWnd, (HMENU)ID_CLEARBTN, NULL, NULL);
            SendMessage(hClearBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hSaveBtn = CreateWindowA("BUTTON", "KAYDET",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_CENTER,
                360, 312, 90, 35, hWnd, (HMENU)ID_SAVEBTN, NULL, NULL);
            SendMessage(hSaveBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hLoadBtn = CreateWindowA("BUTTON", "YUKLE",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_CENTER,
                455, 312, 90, 35, hWnd, (HMENU)ID_LOADBTN, NULL, NULL);
            SendMessage(hLoadBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hClearLogBtn = CreateWindowA("BUTTON", "LOG TEMIZLE",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_CENTER,
                550, 312, 85, 35, hWnd, (HMENU)ID_CLEARLOGBTN, NULL, NULL);
            SendMessage(hClearLogBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Auto-inject checkbox
            hAutoInjectChk = CreateWindowA("BUTTON", "AUTO-INJECT (Otomatik enjeksiyon)",
                WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                15, 355, 300, 20, hWnd, (HMENU)ID_AUTOINJECT, NULL, NULL);
            SendMessage(hAutoInjectChk, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Log label
            CreateWindowA("STATIC", "LOG:",
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                10, 385, 200, 20, hWnd, NULL, NULL, NULL);
            
            // Log kutusu
            hLogBox = CreateWindowExA(WS_EX_CLIENTEDGE, RICHEDIT_CLASSA, "",
                WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | 
                ES_READONLY | WS_VSCROLL,
                10, 405, 630, 150, hWnd, (HMENU)ID_LOGBOX, NULL, NULL);
            SendMessage(hLogBox, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLogBox, EM_SETBKGNDCOLOR, 0, COLOR_BG);
            
            // Status bar
            hStatusBar = CreateWindowA("STATIC", "Hazir - Roblox'u acin ve Enjekte Et'e tiklayin",
                WS_VISIBLE | WS_CHILD | SS_LEFT | SS_SUNKEN,
                0, 562, 650, 22, hWnd, (HMENU)ID_STATUSBAR, NULL, NULL);
            SendMessage(hStatusBar, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Welcome mesajı
            AddLog("========================================", COLOR_CYAN);
            AddLog("   EXECUTOR v3.0 BASLATILDI", COLOR_GREEN);
            AddLog("========================================", COLOR_CYAN);
            AddLog("", COLOR_TEXT);
            AddLog("1) Roblox'u acin", COLOR_BLUE);
            AddLog("2) 'ENJEKTE ET' butonuna tiklayin", COLOR_BLUE);
            AddLog("3) Script yazin ve 'CALISTIR' a tiklayin", COLOR_BLUE);
            AddLog("", COLOR_TEXT);
            AddLog("Hazir bekleniyor...", COLOR_GREEN);
            
            break;
        }
        
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            HWND hControl = (HWND)lParam;
            
            SetBkColor(hdc, COLOR_BG);
            SetTextColor(hdc, COLOR_TEXT);
            
            if(hControl == hStatusBar) {
                SetBkColor(hdc, COLOR_BG3);
            }
            
            return (LRESULT)CreateSolidBrush(COLOR_BG);
        }
        
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, COLOR_BG2);
            SetTextColor(hdc, COLOR_TEXT);
            return (LRESULT)CreateSolidBrush(COLOR_BG2);
        }
        
        case WM_CTLCOLORBTN: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, COLOR_BG3);
            SetTextColor(hdc, COLOR_TEXT);
            return (LRESULT)CreateSolidBrush(COLOR_BG3);
        }
        
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rect;
            GetClientRect(hWnd, &rect);
            HBRUSH brush = CreateSolidBrush(COLOR_BG);
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);
            return 1;
        }
        
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            
            switch(id) {
                case ID_INJECTBTN:
                    CreateThread(NULL, 0, [](LPVOID) -> DWORD {
                        InjectToRoblox();
                        return 0;
                    }, NULL, 0, NULL);
                    break;
                    
                case ID_EXECUTEBTN:
                    CreateThread(NULL, 0, [](LPVOID) -> DWORD {
                        ExecuteScript();
                        return 0;
                    }, NULL, 0, NULL);
                    break;
                    
                case ID_CLEARBTN:
                    ClearScript();
                    break;
                    
                case ID_SAVEBTN:
                    SaveScript();
                    break;
                    
                case ID_LOADBTN:
                    LoadScript();
                    break;
                    
                case ID_CLEARLOGBTN:
                    ClearLog();
                    break;
                    
                case ID_AUTOINJECT: {
                    autoInject = SendMessage(hAutoInjectChk, BM_GETCHECK, 0, 0) == BST_CHECKED;
                    if(autoInject) {
                        AddLog("Auto-Inject AKTIF - Roblox otomatik bulunacak", COLOR_GREEN);
                    } else {
                        AddLog("Auto-Inject PASIF", COLOR_RED);
                    }
                    break;
                }
            }
            break;
        }
        
        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            
            SetWindowPos(hScriptEdit, NULL, 10, 75, width - 25, 220, SWP_NOZORDER);
            SetWindowPos(hLogBox, NULL, 10, 405, width - 25, height - 470, SWP_NOZORDER);
            SetWindowPos(hStatusBar, NULL, 0, height - 22, width, 22, SWP_NOZORDER);
            break;
        }
        
        case WM_DESTROY:
            injector.Detach();
            DeleteObject(hFont);
            PostQuitMessage(0);
            break;
    }
    
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// WinMain giriş noktası
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Anti-debug başlat
    AntiDebug::Initialize();
    
    // Rich edit kütüphanesini yükle
    LoadLibraryA("msftedit.dll");
    
    // Pencere sınıfı kaydet
    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(COLOR_BG);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "ExecutorClass";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    if(!RegisterClassExA(&wc)) {
        MessageBoxA(NULL, "Pencere sinifi kaydedilemedi!", "Hata", MB_ICONERROR);
        return 1;
    }
    
    // Ana pencere oluştur
    hMainWnd = CreateWindowExA(
        WS_EX_APPWINDOW,
        "ExecutorClass",
        "Executor v3.0 - Advanced Script Executor",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        (GetSystemMetrics(SM_CXSCREEN) - 650) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - 620) / 2,
        650, 620,
        NULL, NULL, hInstance, NULL
    );
    
    if(!hMainWnd) {
        MessageBoxA(NULL, "Pencere olusturulamadi!", "Hata", MB_ICONERROR);
        return 1;
    }
    
    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);
    
    // Auto-inject thread başlat
    std::thread autoThread(AutoInjectLoop);
    autoThread.detach();
    
    // Mesaj döngüsü
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
