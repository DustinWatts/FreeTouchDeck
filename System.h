#pragma once
#include <cJSON.h>
#include "esp_attr.h"


namespace FreeTouchDeck
{

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
    void PrintMemInfo();
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
    void processSleep();
    bool getTouch(uint16_t *t_x, uint16_t *t_y);
    bool isTouched();
    void WaitTouchReboot();    
    void ResetSleep();
    IRAM_ATTR char *ps_strdup(const char *fmt);
    extern SystemMode restartReason;
    extern SystemMode RunMode;
}