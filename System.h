#pragma once
#include <cJSON.h>
#include "esp_attr.h"
#include "BleKeyboard.h"
#include "UserConfig.h"
#include "FTAction.h"
namespace FreeTouchDeck
{
    extern BleKeyboard bleKeyboard;
    enum class SystemMode
    {
        STANDARD,
        CONSOLE,
        CONFIG
    };

    void DumpCJson(cJSON *doc);
    void touchInit();
    void SetSleepInterval(unsigned long sleepInterval);
    const char *enum_to_string(SystemMode mode);
    void LoadSystemConfig();
    void PrintBasicMemInfo();
    void PrintMemStats();
    void StopBluetooth();
#ifdef PRINT_MEM_INFO

    void PrintMemInfo(const char * fn, uint16_t line);
  #else
    #define PrintMemInfo(fn,line)
#endif        
    void TFTPrintMemInfo();
    void powerInit();
    void InitSystem();
    void HandleSleepConfig();
    void HandleBeepConfig();
    void ChangeMode(SystemMode newMode);
    void *malloc_fn(size_t sz);
    bool EnterSleep();
    void HandleActions();
    void HandleScreen();
    char * AllocPrintJson(cJSON * doc, bool freeDoc = true);
    bool SaveJsonToFile(const char * fileName,cJSON * content,bool freeDoc=true);
    void processSleep();
    bool getTouch(uint16_t *t_x, uint16_t *t_y);
    bool isTouched();
    void WaitTouchReboot();    
    void ResetSleep();
    char *ps_strdup(const char *fmt);
    extern SystemMode restartReason;
    extern SystemMode RunMode;
}