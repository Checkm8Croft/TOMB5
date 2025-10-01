#include "registry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_FILE ".tomb5_config.ini"

static int keyCreated = 0;
static int REG_Setup = 0;

typedef struct {
    char key[128];
    char value[128];
} ConfigEntry;

static ConfigEntry entries[256];
static int entryCount = 0;
static char configPath[512];

// ---------------- FUNZIONI DI BASE ----------------

static void loadConfigFile() {
    entryCount = 0;
    FILE* f = fopen(configPath, "r");
    if (!f) {
        keyCreated = 1;
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char* eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        char* key = line;
        char* val = eq + 1;

        // togli \n
        val[strcspn(val, "\r\n")] = '\0';

        strncpy(entries[entryCount].key, key, sizeof(entries[entryCount].key) - 1);
        strncpy(entries[entryCount].value, val, sizeof(entries[entryCount].value) - 1);
        entryCount++;
    }
    fclose(f);
}

static void saveConfigFile() {
    FILE* f = fopen(configPath, "w");
    if (!f) return;
    for (int i = 0; i < entryCount; i++) {
        fprintf(f, "%s=%s\n", entries[i].key, entries[i].value);
    }
    fclose(f);
}

static const char* getValue(const char* key) {
    for (int i = 0; i < entryCount; i++) {
        if (strcmp(entries[i].key, key) == 0)
            return entries[i].value;
    }
    return NULL;
}

static void setValue(const char* key, const char* value) {
    for (int i = 0; i < entryCount; i++) {
        if (strcmp(entries[i].key, key) == 0) {
            strncpy(entries[i].value, value, sizeof(entries[i].value) - 1);
            return;
        }
    }
    if (entryCount < 256) {
        strncpy(entries[entryCount].key, key, sizeof(entries[entryCount].key) - 1);
        strncpy(entries[entryCount].value, value, sizeof(entries[entryCount].value) - 1);
        entryCount++;
    }
}

// ---------------- API COMPATIBILI ----------------

bool OpenRegistry(const char* SubKeyName) {
    const char* home = getenv("HOME");
    if (!home) home = ".";
    snprintf(configPath, sizeof(configPath), "%s/%s", home, CONFIG_FILE);
    loadConfigFile();
    return 1;
}

void CloseRegistry() {
    saveConfigFile();
}

bool REG_KeyWasCreated() {
    return keyCreated;
}

void REG_WriteLong(const char* SubKeyName, unsigned long value) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%lu", value);
    setValue(SubKeyName, buf);
}

void REG_WriteBool(const char* SubKeyName, bool value) {
    setValue(SubKeyName, value ? "1" : "0");
}

bool REG_ReadLong(const char* SubKeyName, unsigned long* value, unsigned long defaultValue) {
    const char* v = getValue(SubKeyName);
    if (v) {
        *value = strtoul(v, NULL, 10);
        return 1;
    }
    *value = defaultValue;
    REG_WriteLong(SubKeyName, defaultValue);
    return 0;
}

bool REG_ReadBool(const char* SubKeyName, bool* value, bool defaultValue) {
    const char* v = getValue(SubKeyName);
    if (v) {
        *value = (strcmp(v, "1") == 0);
        return 1;
    }
    *value = defaultValue;
    REG_WriteBool(SubKeyName, defaultValue);
    return 0;
}

// ---------------- MOCK FUNZIONI ORIGINALI ----------------

bool LoadSettings() {
    if (!OpenRegistry("System"))
        return 0;

    REG_ReadBool("Setup", (bool*)&REG_Setup, 0);

    CloseRegistry();
    return REG_Setup;
}

void SaveSettings() {
    OpenRegistry("Game");
    // esempio: REG_WriteLong("MusicVolume", 80);
    CloseRegistry();
}

bool SaveSetup(void* hDlg) {
    OpenRegistry("System");
    REG_WriteBool("Setup", 1);
    CloseRegistry();
    return 1;
}
