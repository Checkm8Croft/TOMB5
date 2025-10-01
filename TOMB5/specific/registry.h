#pragma once

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool OpenRegistry(const char* SubKeyName);
void CloseRegistry();

bool REG_KeyWasCreated();
void REG_WriteLong(const char* SubKeyName, unsigned long value);
void REG_WriteBool(const char* SubKeyName, bool value);
bool REG_ReadLong(const char* SubKeyName, unsigned long* value, unsigned long defaultValue);
bool REG_ReadBool(const char* SubKeyName, bool* value, bool defaultValue);

bool LoadSettings();
void SaveSettings();
bool SaveSetup(void* hDlg); // ignorato su macOS/Linux

#ifdef __cplusplus
}
#endif
