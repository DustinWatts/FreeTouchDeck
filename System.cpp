#include "globals.hpp"
#include "UserConfig.h"
#include "System.h"
#include "MenuNavigation.h"
#include "Storage.h"
#include "ConfigLoad.h"
#include "ConfigHelper.h"
#include "Audio.h"
#include "UserConfig.h"

#ifdef USECAPTOUCH
#include "CapacitiveTouch.h"
#else
#include "ResistiveTouch.h"
#endif
namespace FreeTouchDeck
{
    static const char *module = "System";
    volatile unsigned long previousMillis = 0;
    unsigned long SleepInterval = 0;
    BleKeyboard bleKeyboard("FreeTouchDeck", "Made by me");
    RTC_NOINIT_ATTR SystemMode restartReason = SystemMode::STANDARD;
    SystemMode RunMode = SystemMode::STANDARD;

    IRAM_ATTR char *ps_strdup(const char *fmt)
    {
        // Duplicate string values, calling our own
        // memory allocation so we can decide to use
        // PSRAM or not
        const char *s = NULL;
        if (fmt && strlen(fmt) > 0)
        {
            s = fmt;
        }
        else
        {
            s = "";
        }
        char *o = (char *)malloc_fn(strlen(fmt) + 1);
        if (o)
        {
            memcpy(o, fmt, strlen(fmt));
        }
        return o;
    }
    void StopBluetooth()
    {
        bleKeyboard.end();
        LOC_LOGD(module, "Terminating BT");
        // Stop BLE from interfering with our WIFI signal
        btStop();
        // esp_bt_controller_disable();
        // esp_bt_controller_deinit();
        // esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);
        LOC_LOGI(module, "BLE Stopped");
    }
    void *malloc_fn(size_t sz)
    {
        void *ptr = NULL;
        ptr = malloc(sz);
        if (!ptr)
        {
            LOC_LOGE(module, "Memory allocation failed for %d bytes. free_iram: %d ", sz, heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
            drawErrorMessage(true, module, "Memory allocation failed");
        }
        else
        {
            memset(ptr, 0x00, sz);
        }
        return ptr;
    }

    void init_cJSON()
    {
        static cJSON_Hooks hooks;
        hooks.malloc_fn = &malloc_fn;
        cJSON_InitHooks(&hooks);
    }
    char *AllocPrintJson(cJSON *doc, bool freeDoc)
    {
        char *outStr = cJSON_Print(doc);
        if (freeDoc)
        {
            cJSON_Delete(doc);
        }
        if (!outStr)
        {
            LOC_LOGE(module, "Error generating text for json");
            return NULL;
        }
        return outStr;
    }
    bool SaveJsonToFile(const char *fileName, cJSON *content, bool freeDoc)
    {
        PrintMemInfo(__FUNCTION__, __LINE__);
        char *contentString = AllocPrintJson(content, freeDoc);
        bool result = false;
        if (!ISNULLSTRING(contentString))
        {
            fs::File file = ftdfs->open(fileName, "w");
            if (!file)
            {
                LOC_LOGE(module, "Error opening target file %s", STRING_OR_DEFAULT(fileName, "?"));
            }
            else
            {
                size_t written = file.write((uint8_t *)contentString, strlen(contentString));
                file.close();
                if (written != strlen(contentString))
                {
                    LOC_LOGE(module, "File %s could not be fully written to. Wrote %d bytes of %d bytes", STRING_OR_DEFAULT(fileName, "?"), written, strlen(contentString));
                }
                else
                {
                    result = true;
                }
            }
            FREE_AND_NULL(contentString);
        }
        PrintMemInfo(__FUNCTION__, __LINE__);
        return result;
    }
    void InitSystem()
    {
        RESET_REASON resetReason = rtc_get_reset_reason(0);
        esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
        init_cJSON();
        PrintMemInfo(__FUNCTION__, __LINE__);
        touchInit();
        PrintMemInfo(__FUNCTION__, __LINE__);
        InitFileSystem();
        PrintMemInfo(__FUNCTION__, __LINE__);

        // We cannot rely on the c++ compiler to initialize our
        // contants, for example the buttons list which is required by
        // other constants.  Initializing them here ensure that
        // primitive maps will exist before we try to access them
        FTAction::InitConstants();
        FTButton::InitConstants();
        PrintMemInfo(__FUNCTION__, __LINE__);
        LoadSystemConfig();
        PrintMemInfo(__FUNCTION__, __LINE__);
        // Init display
        displayInit();

        // ------------------- Determine system mode  ------------------
        if (restartReason != SystemMode::STANDARD && restartReason != SystemMode::CONFIG && restartReason != SystemMode::CONSOLE)
        {
            restartReason = SystemMode::STANDARD;
        }
        LOC_LOGI(module, "Found Reset reason: %d", resetReason);
        LOC_LOGI(module, "System mode: %s", enum_to_string(restartReason));

        if ((resetReason == SW_RESET || resetReason == SW_CPU_RESET))
        {
            //reset restartreason for next reboot

            if (restartReason == SystemMode::CONFIG)
            {
                generalconfig.sleepenable = false;
                if (ConfigMode())
                {
                    RunMode = SystemMode::CONFIG;
                    restartReason = SystemMode::STANDARD;
                    LoadAllMenus();
                    return;
                }
                else
                {
                    drawErrorMessage(true, module, "Unable to start config mode. Please reset device.");
                }
            }
            else if (restartReason == SystemMode::CONSOLE)
            {
                RunMode = SystemMode::CONSOLE;
                restartReason = SystemMode::STANDARD;
                PrintScreenMessage(true, "Console mode active. Press screen to exit");
                LoadAllMenus();
                return;
            }
        }

        if (wakeup_reason > ESP_SLEEP_WAKEUP_UNDEFINED)
        {
            // If we are woken up we do not need the splash screen
            // But we do draw something to indicate we are waking up
            SetSmallestFont(1);
            tft.println(" Waking up...");
        }
        else
        {
            // Draw a splash screen
            LOC_LOGD(module, "Displaying version details");
            ClearScreen();
            DrawSplash();
            tft.printf("Loading version %s\n", versionnumber);
            LOC_LOGI(module, "Loading version %s", versionnumber);
        }

        HandleAudio(Sounds::STARTUP);
        // Calibrate the touch screen and retrieve the scaling factors
        touch_calibrate();
        LoadAllMenus();
        LOC_LOGI(module, "All config files loaded");
        //------------------BLE Initialization ------------------------------------------------------------------------
        LOC_LOGI(module, "Starting BLE Keyboard");
        bleKeyboard.deviceName = generalconfig.deviceName;
        bleKeyboard.deviceManufacturer = generalconfig.manufacturer;
        bleKeyboard.begin();
        PrintMemInfo(__FILE__, __LINE__);
    }

    void ChangeMode(SystemMode newMode)
    {
        restartReason = newMode;
        ESP.restart();
    }

    const char *enum_to_string(SystemMode mode)
    {
        switch (mode)
        {

            ENUM_TO_STRING_HELPER(SystemMode, STANDARD);
            ENUM_TO_STRING_HELPER(SystemMode, CONSOLE);
            ENUM_TO_STRING_HELPER(SystemMode, CONFIG);
        default:
            return "unknown";
        }
    }
    void ResetSleep()
    {
        previousMillis = millis();
    }
    void LoadSystemConfig()
    {
        // After checking the config files exist, actually load them
        if (!loadGeneralConfig())
        {
            drawErrorMessage(false, module, "general.json seems to be corrupted. To reset to default type 'reset general'.");
        }
        //------------------ Load Wifi Config ----------------------------------------------
        PrintMemInfo(__FUNCTION__, __LINE__);
        LOC_LOGI(module, "Loading Wifi Config");
        if (!loadWifiConfig())
        {
            LOC_LOGW(module, "Failed to load WiFi Credentials!");
        }
        else
        {
            LOC_LOGI(module, "WiFi Credentials Loaded");
        }
        PrintMemInfo(__FUNCTION__, __LINE__);
    }
    void PrintMemStats()
    {
        if (generalconfig.memStatsDelay == 0) {
            return;
        }
        static unsigned long t = 0;
        if(millis() >t)
        {
            PrintBasicMemInfo();
            t=millis()+generalconfig.memStatsDelay;
        }
    }
    void PrintBasicMemInfo()
    {
        static size_t prev_free = 0;
        static size_t prev_free_8 = 0;
        static size_t prev_free_32 = 0;
        Serial.printf("Memory: free ram 32bits[f:%d,l:%d,d:%d], 8bits[f:%d,l:%d,d:%d], internal:[f:%d,l:%d,d:%d] \n",
                      heap_caps_get_free_size(MALLOC_CAP_32BIT), heap_caps_get_largest_free_block(MALLOC_CAP_32BIT), prev_free_32 > 0 ? prev_free_32 - heap_caps_get_free_size(MALLOC_CAP_32BIT) : 0,
                      heap_caps_get_free_size(MALLOC_CAP_8BIT), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT), prev_free_8 > 0 ? prev_free_8 - heap_caps_get_free_size(MALLOC_CAP_8BIT) : 0,
                      heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL), prev_free > 0 ? prev_free - heap_caps_get_free_size(MALLOC_CAP_INTERNAL) : 0);
        prev_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        prev_free_8 = heap_caps_get_free_size(MALLOC_CAP_8BIT);
        prev_free_32 = heap_caps_get_free_size(MALLOC_CAP_32BIT);
    }
#ifdef PRINT_MEM_INFO

    void PrintMemInfo(const char *fn, uint16_t line)
    {

        static size_t prev_free = 0;
        static size_t prev_free_8 = 0;
        static size_t prev_free_32 = 0;
        if(generalconfig.LogLevel <LogLevels::VERBOSE) return;
        Serial.printf("%s[%d]free ram 32bits[f:%d,l:%d,d:%d], 8bits[f:%d,l:%d,d:%d], internal:[f:%d,l:%d,d:%d] \n",
                      fn, line, heap_caps_get_free_size(MALLOC_CAP_32BIT), heap_caps_get_largest_free_block(MALLOC_CAP_32BIT), prev_free_32 > 0 ? prev_free_32 - heap_caps_get_free_size(MALLOC_CAP_32BIT) : 0,
                      heap_caps_get_free_size(MALLOC_CAP_8BIT), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT), prev_free_8 > 0 ? prev_free_8 - heap_caps_get_free_size(MALLOC_CAP_8BIT) : 0,
                      heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL), prev_free > 0 ? prev_free - heap_caps_get_free_size(MALLOC_CAP_INTERNAL) : 0);
        prev_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        prev_free_8 = heap_caps_get_free_size(MALLOC_CAP_8BIT);
        prev_free_32 = heap_caps_get_free_size(MALLOC_CAP_32BIT);
    }
#else
#define PrintMemInfo(fn, line)
#endif
    void TFTPrintMemInfo()
    {
        tft.printf("free ram 32bits[%d,%d], 8bits[%d,%d], internal:[%d,%d] \n",
                   heap_caps_get_free_size(MALLOC_CAP_32BIT), heap_caps_get_largest_free_block(MALLOC_CAP_32BIT),
                   heap_caps_get_free_size(MALLOC_CAP_8BIT), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
                   heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    }
    void WaitTouchReboot()
    {
        PrintScreenMessage(false, "Press screen to restart");
        while (true)
        {
            if (isTouched())
            {
                PrintScreenMessage(false, "Restarting...");
                delay(2000);
                ESP.restart();
            }
            yield();
        }
    }
    void powerInit()
    {
#ifdef ARDUINO_TWATCH_BASE
        LOC_LOGI(module, "Enabling AXP power management chip.");
        Wire1.begin(21, 22);
        int ret = power->begin(Wire1, AXP202_SLAVE_ADDRESS, false);
        if (ret == AXP_FAIL)
        {
            LOC_LOGE(module, "AXP Power begin failed");
        }
        else
        {
            // todo: implement better power management for
            // watch
            //Change the shutdown time to 4 seconds
            //power->setShutdownTime(AXP_POWER_OFF_TIME_4S);
            // Turn off the charging instructions, there should be no
            //power->setChgLEDMode(AXP20X_LED_OFF);
            // Turn off external enable
            //power->setPowerOutPut(AXP202_EXTEN, false);
            //axp202 allows maximum charging current of 1800mA, minimum 300mA
            power->setChargeControlCur(300);
        }
        power->setPowerOutPut(AXP202_LDO2, AXP202_ON);
        LOC_LOGI(module, "Setting up Display Back light");
#endif
        // Setup PWM channel and attach pin 32
        ledcSetup(0, 5000, 8);
#ifdef TFT_BL
        ledcAttachPin(TFT_BL, 0);
#else
        ledcAttachPin(32, 0);
#endif
        ledcWrite(0, generalconfig.ledBrightness); // Start @ initial Brightness
    }

    void HandleSleepConfig()
    {

        if (generalconfig.sleepenable && touchInterruptPin >= 0)
        {
            // todo: implement sleep logic
            pinMode(touchInterruptPin, INPUT_PULLUP);
            SetSleepInterval(generalconfig.sleeptimer);
            QueueAction(sleepSetLatchAction);
            LOC_LOGI(module, "Sleep enabled. Timer = %d minutes", generalconfig.sleeptimer);
        }
        else
        {
            QueueAction(sleepClearLatchAction);
        }
    }
    void HandleBeepConfig()
    {
        if (generalconfig.beep)
        {
            QueueAction(beepSetLatchAction);
        }
        else
        {
            QueueAction(beepClearLatchAction);
        }
    }
    void touchInit()
    {

        PrintMemInfo(__FUNCTION__, __LINE__);
#ifdef USECAPTOUCH
#ifdef CUSTOM_TOUCH_SDA
        if (!ts.begin(40, CUSTOM_TOUCH_SDA, CUSTOM_TOUCH_SCL))
#else
        if (!ts.begin(40))
#endif
        {
            LOC_LOGE(module, "Unable to start the capacitive touchscreen.");
        }
        else
        {
            LOC_LOGI(module, "Capacitive touch started");
        }
#endif
        PrintMemInfo(__FUNCTION__, __LINE__);
    }
    void DumpCJson(cJSON *doc)
    {
        if (generalconfig.LogLevel < LogLevels::VERBOSE)
            return;
        char *d = cJSON_Print(doc);
        if (d)
        {
            LOC_LOGD(module, "%s", d);
        }
        FREE_AND_NULL(d);
    }
#ifdef USECAPTOUCH
    bool getTouch(uint16_t *t_x, uint16_t *t_y)
    {
        static bool prev = false;
        if (ts.touched())
        {
            // Retrieve a point
            TS_Point p = ts.getPoint();
            if (generalconfig.flip_touch_axis)
            {
                //Flip things around so it matches our screen rotation
                *t_y = p.x;
                *t_x = p.y;
            }
            else
            {
                *t_y = p.y;
                *t_x = p.x;
            }

            if (generalconfig.reverse_x_touch)
            {
                *t_x = (uint16_t)map((long)*t_x, (long)0, (long)tft.width(), (long)tft.width(), (long)0);
            }
            if (generalconfig.reverse_y_touch)
            {
                *t_y = (uint16_t)map((long)*t_y, (long)0, (long)tft.height(), (long)tft.height(), (long)0);
            }

            if (!prev)
            {
                LOC_LOGD(module, "Input touch (%dx%d)=>(%dx%d) - Screen is %dx%d", p.x, p.y, *t_x, *t_y, tft.width(), tft.height());
                prev = true;
            }

            ResetSleep();
            return true;
        }
        prev = false;
        return false;
    }
#else
    bool getTouch(uint16_t *t_x, uint16_t *t_y)
    {
        if (tft.getTouch(t_x, t_y))
        {
            ResetSleep();
            return true;
        }
        return false;
    }
#endif
    bool isTouched()
    {
        uint16_t t_x, t_y;
        return getTouch(&t_x, &t_y);
    }
    void processSleep()
    {
        if (RunMode == SystemMode::CONSOLE || RunMode == SystemMode::CONFIG)
        {
            // console mode and config mode don't require sleep
            return;
        }
        if (generalconfig.sleepenable && touchInterruptPin >= 0)
        {
            if (millis() > previousMillis + SleepInterval)
            {
                // The timer has ended and we are going to sleep  .
                EnterSleep();
            }
        }
    }
    bool EnterSleep()
    {
        // esp_deep_sleep does not shut down WiFi, BT, and higher level protocol connections gracefully. Make sure relevant WiFi and BT stack functions are called to close any connections and deinitialize the peripherals. These include:
        tft.fillScreen(TFT_BLACK);
        LOC_LOGD(module, "Going to sleep.");
        HandleAudio(Sounds::GOING_TO_SLEEP);
        //todo better power management for TWATCH
        //       power->setPowerOutPut(AXP202_LDO2, AXP202_OFF);
        esp_err_t err = esp_sleep_enable_ext0_wakeup(touchInterruptPin, 0);
        if (err != ESP_OK)
        {
            LOC_LOGE(module, "Unable to set external wakeup pin: %s", esp_err_to_name(err));
            generalconfig.sleepenable = false;
            //
            HandleSleepConfig();
            // disable sleep for next time
            QueueSaving();
            return false;
        }
        else
        {

            esp_deep_sleep_start();
        }
        return true;
    }
    void SetSleepInterval(unsigned long sleepInterval)
    {
        // sleep interval is expressed in minutes
        SleepInterval = sleepInterval * 60000;
    }

    void HandleScreen()
    {
        uint16_t t_x = 0;
        uint16_t t_y = 0;
        try
        {
            bool pressed = getTouch(&t_x, &t_y);
            if (RunMode == SystemMode::CONFIG || RunMode == SystemMode::CONSOLE)
            {
                delay(100);
                if (pressed)
                {
                    ESP.restart();
                }
                return;
            }
            handleDisplay(pressed, t_x, t_y);
            LOC_LOGV(module, "Checking for screen actions");
            FTAction *Action = PopScreenQueue();
            if (Action)
            {
                ResetSleep();
                Action->Execute();
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    void HandleActions()
    {
        LOC_LOGV(module, "Checking for regular actions");
        try
        {
            FTAction *Action = NULL;
            Action = PopQueue();
            if (Action)
            {
                ResetSleep();
                Action->Execute();
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
};

// Overloading global new operator
void *operator new(size_t sz)
{
    void *m = FreeTouchDeck::malloc_fn(sz);
    return m;
}