#include "stdafx.h"
#include "../shared/Condition.h"
#include "ConsoleInputThread.h"
#include "../shared/signal_handler.h"
#include "../shared/CrashHandler.h"
#include "GameServerDlg.h" // SetConsoleColor fonksiyonunu i�eren ba�l�k dosyas�
#include <psapi.h> // GetProcessMemoryInfo fonksiyonu i�in gerekli

CGameServerDlg* g_pMain;
static Condition s_hEvent;

BOOL WINAPI _ConsoleHandler(DWORD dwCtrlType);

bool g_bRunning = true;



int main()
{
    HWND console = GetConsoleWindow();
    RECT ConsoleRect;
    GetWindowRect(console, &ConsoleRect);

    MoveWindow(console, ConsoleRect.left, ConsoleRect.top, 900, 400, TRUE);

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);

    uint16 m_bOnlineCount = 0, m_bMerchantCount = 0, m_bMiningCount = 0, m_bGenieCount = 0, m_bFishingCount = 0;

    SIZE_T physMemUsedByMe = g_pMain->GetProcessMemoryUsage(); // Bellek kullan�m�n� al
    std::string texts;
    texts = string_format("ISTIRAP (Game Server) [Online :%d - Merchant :%d - Mining :%d - Fishing :%d - Genie :%d] || RAM: %llu MB",
        m_bOnlineCount, m_bMerchantCount, m_bMiningCount, m_bFishingCount, m_bGenieCount, physMemUsedByMe);
    SetConsoleTitle(texts.c_str());

    // 20.10.2020 Gameserver a��l�� s�resi hesaplama start
    DateTime time;
    clock_t x = clock();
    // 20.10.2020 Gameserver a��l�� s�resi hesaplama end
    CCrashHandler XxX; // x
    XxX.SetProcessExceptionHandlers(); // x
    XxX.SetThreadExceptionHandlers(); // x

    // Override the console handler
    SetConsoleCtrlHandler(_ConsoleHandler, TRUE);

    HookSignals(&s_hEvent);

    // Start up the time updater thread
    StartTimeThread();

    ExplosionHandle::SetupExceptionHandler(); // BugTrap 27.09.2020

    // Start up the console input thread
    StartConsoleInputThread();

    g_pMain = new CGameServerDlg();
    g_pMain->s_hEvent = &s_hEvent;

    // Start up server
    if (g_pMain->Startup())
    {
        // Reset Battle Zone Variables.
        g_pMain->ResetBattleZone(BATTLEZONE_NONE);

        g_pMain->SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Yellow color
        printf("***********************************************\n");
        printf("* Serveriniz Basarili Olarak Baslatildi Oyuna Giris Aktif.\n");
        printf("** > GameServer Baslama Hizi (%.2lf Seconds) on %04d-%02d-%02d at %02d:%02d\n",
            (clock() - x) / (double)CLOCKS_PER_SEC, time.GetYear(), time.GetMonth(),
            time.GetDay(), time.GetHour(), time.GetMinute());
        printf("* Eglenceli Oyunlar Dilerim\n");
        printf("***********************************************\n");
        g_pMain->SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default color

        // Wait until console's signaled as closing
        s_hEvent.Wait();
    }
    else
    {
        system("pause");
    }

    g_pMain->SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    printf("Server is shutting down, please wait...\n");
    g_pMain->SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    g_bRunning = false;

    delete g_pMain;

    CleanupTimeThread();
    CleanupConsoleInputThread();
    UnhookSignals();

    return 0;
}

BOOL WINAPI _ConsoleHandler(DWORD dwCtrlType)
{
    s_hEvent.BeginSynchronized();
    s_hEvent.Signal();
    s_hEvent.EndSynchronized();
    sleep(10000); // Win7 onwards allows 10 seconds before it'll forcibly terminate
    return TRUE;
}
