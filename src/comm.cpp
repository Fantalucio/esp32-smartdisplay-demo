

#include <stdio.h>
#include <stdint.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <WiFi.h>

#include <Arduino.h>

#include <esp32_smartdisplay.h>
#include <ui/ui.h>

#include "comm.h"

WIFI_taskData_t WIFI_taskData;
CONN_taskData_t CONN_taskData;

WIFI_taskData_t *wifiTaskInit(void)
{
    WIFI_taskData.function = WIFI_INIT;
    WIFI_taskData.oldfunction = WIFI_FUNC_MAX;
    //    WIFI_taskData.statusFlags = BIT_START_INIT;
    return &WIFI_taskData;
}

void eventDisconnect_cb()
{
    WIFI_taskData.statusFlags &= ~BIT_CONNECT;
}

void eventConnect_cb()
{
    WIFI_taskData.statusFlags |= BIT_CONNECT;
}

/*
    MANAGE WIFI
*/
void manage_WIFI(void)
{
    int intTemp;
    SYS_TICK_t tickNow = millis();

    switch (WIFI_taskData.function)
    {
    case WIFI_INIT:
        if (WIFI_taskData.oldfunction != WIFI_taskData.function)
        {
            WIFI_taskData.oldfunction = WIFI_taskData.function;
            WIFI_taskData.tickStart = tickNow;

            WiFi.mode(WIFI_STA);
            WiFi.disconnect();

            log_i("WIFI_TASK INIT");
        }

        if (tickNow - WIFI_taskData.tickStart > 500)
        {
            WIFI_taskData.function = WIFI_WAIT;
            WIFI_taskData.statusFlags = BIT_INIT_OK;
        }

        break;

    case WIFI_WAIT:
        if (WIFI_taskData.oldfunction != WIFI_taskData.function)
        {
            WIFI_taskData.oldfunction = WIFI_taskData.function;
            WIFI_taskData.tickStart = millis();
            log_i("WIFI_TASK WAIT");
        }

        break;

    case WIFI_SCAN:
        if (WIFI_taskData.oldfunction != WIFI_taskData.function)
        {
            WIFI_taskData.oldfunction = WIFI_taskData.function;
            WIFI_taskData.tickStart = millis();
            WIFI_taskData.tickWait = 2000; /*scansione ogni 2 secondi*/

            WiFi.scanNetworks(true);
            log_i("WIFI_TASK SCAN");
            WIFI_taskData.numAP = 0;
        }

        // WiFi.scanNetworks will return the number of networks found
        intTemp = WiFi.scanComplete();

        if (intTemp == WIFI_SCAN_FAILED)
        {
            if (intTemp == WIFI_SCAN_FAILED)
            {
                log_i("WIFI_TASK scan failed");
            }
            else
            {
                log_i("WIFI_TASK scan timeout");
            }

            WIFI_taskData.statusFlags |= BIT_SCAN_ERROR;
            WIFI_taskData.function = WIFI_WAIT;
            //            WiFi.scanDelete();
        }
        else if (intTemp >= 0)
        {
            log_i("WIFI_TASK scan completed");

            WIFI_taskData.numAP = intTemp;

            // char text_buffer[32];
            // /*
            //             String ssidString = WiFi.SSID(0);
            //             ssidString.toCharArray(text_buffer, ssidString.indexOf("\n"));
            //             lv_label_set_text(ui_lblWifiSsidValue, text_buffer);
            //  */
            // // Serial.printf("Found %d networks\n", n);
            // for (int i = 0; i < intTemp; ++i)
            // {
            //     sprintf(text_buffer, "%d: %s, RSSI: %d\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
            //     lv_list_add_btn(ui_wifiList, LV_SYMBOL_WIFI, text_buffer);
            // }
            WIFI_taskData.statusFlags = BIT_SCAN_COMPLETED;
            WIFI_taskData.statusFlags &= ~BIT_SCAN_ERROR;
            WIFI_taskData.function = WIFI_WAIT;
            //            WiFi.scanDelete();
        }

        break;

    case WIFI_CONNECTION:
        if (WIFI_taskData.oldfunction != WIFI_taskData.function)
        {
            WIFI_taskData.oldfunction = WIFI_taskData.function;
            WIFI_taskData.tickStart = millis();

            //  WiFi.onEvent(eventDisconnect_cb, WIFI_EVENT_STA_DISCONNECTED);
            // WiFi.onEvent(eventConnect_cb, WIFI_EVENT_STA_CONNECTED);
            log_i("WIFI_TASK connection");
        }

        break;

    case WIFI_RECONNECTION:
        if (WIFI_taskData.oldfunction != WIFI_taskData.function)
        {
            WIFI_taskData.oldfunction = WIFI_taskData.function;
            WIFI_taskData.tickStart = millis();
            log_i("WIFI_TASK reconnection");
        }
        break;

    default:
        wifiTaskInit();
    }
}



static void AP_list_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        LV_UNUSED(obj);
        LV_LOG_USER("Clicked: %s", lv_list_get_btn_text(ui_AP_list, obj));

        lv_label_set_text(ui_lblWifiSsidValue, lv_list_get_btn_text(ui_AP_list, obj));
    }
}


CONN_taskData_t *CONN_taskInit(void)
{
    CONN_taskData.function = CONN_INIT;
    CONN_taskData.oldfunction = CONN_FUNC_MAX;
    CONN_taskData.statusFlags = 0;
    return &CONN_taskData;
}

void manage_connection(void)
{

    static lv_obj_t *AP_list[C_AP_LIST_MAX] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}; // elenco bottoni aggiunti alla lista

    SYS_TICK_t tickNow = millis();

    switch (CONN_taskData.function)
    {
    case CONN_INIT:
        if (CONN_taskData.oldfunction != CONN_taskData.function)
        {
            CONN_taskData.oldfunction = CONN_taskData.function;
            CONN_taskData.tickStart = tickNow;

            log_i("CONN_TASK INIT");

            if (WIFI_taskData.statusFlags & BIT_SCAN_ERROR)
            {
                WIFI_taskData.function = WIFI_INIT;
            }
        }

        if (tickNow - CONN_taskData.tickStart > 1000)
        {
            if (WIFI_taskData.statusFlags == BIT_INIT_OK)
            {
                CONN_taskData.function = CONN_WAIT;
                CONN_taskData.statusFlags = BIT_INIT_OK;
            }
        }

        break;

    case CONN_WAIT:
        if (CONN_taskData.oldfunction != CONN_taskData.function)
        {
            CONN_taskData.oldfunction = CONN_taskData.function;
            CONN_taskData.tickStart = tickNow;
            CONN_taskData.tickWait = 5000;
            log_i("CONN_TASK WAIT");
        }

        if (WIFI_taskData.function == WIFI_WAIT && tickNow - CONN_taskData.tickStart > CONN_taskData.tickWait)
        {
            CONN_taskData.function = CONN_SCAN;
        }

        break;

    case CONN_SCAN:
        if (CONN_taskData.oldfunction != CONN_taskData.function)
        {
            CONN_taskData.oldfunction = CONN_taskData.function;
            CONN_taskData.tickStart = tickNow;
            CONN_taskData.tickWait = 5000; // carico scan timeout

            // cancello gli oggetti che compongono la lista
            for (int i = 0; i < C_AP_LIST_MAX; i++)
            {
                if (AP_list[i] != NULL)
                {
                    lv_obj_del(AP_list[i]);
                    AP_list[i] = NULL;
                }
            }

            WIFI_taskData.function = WIFI_SCAN;
            log_i("CONN_TASK SCAN");
        }

        if (WIFI_taskData.statusFlags == BIT_SCAN_COMPLETED)
        {
            log_i("CONN_TASK scan completato");

            char text_buffer[65]; // SSID può essere lungo 64 caratteri

            for (int i = 0; i < WIFI_taskData.numAP; ++i)
            {
                if (i < C_AP_LIST_MAX)
                {
                    // sprintf(text_buffer, "%d: %s, RSSI: %d\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
                    sprintf(text_buffer, "%s\n", WiFi.SSID(i).c_str());
                    AP_list[i] = lv_list_add_btn(ui_AP_list, LV_SYMBOL_WIFI, text_buffer);
                    lv_obj_add_event_cb(AP_list[i], AP_list_event_handler, LV_EVENT_CLICKED, ui_AP_list);
                }
            }
            CONN_taskData.function = CONN_WAIT;
        }
        else if (tickNow - CONN_taskData.tickStart > CONN_taskData.tickWait)
        {
            log_i("CONN_TASK scan timeout");
            CONN_taskData.function = CONN_WAIT;
        }

        break;

    case CONN_CONNECTION:
        if (CONN_taskData.oldfunction != CONN_taskData.function)
        {
            CONN_taskData.oldfunction = CONN_taskData.function;
            CONN_taskData.tickStart = tickNow;

            //  WiFi.onEvent(eventDisconnect_cb, WIFI_EVENT_STA_DISCONNECTED);
            // WiFi.onEvent(eventConnect_cb, WIFI_EVENT_STA_CONNECTED);
            log_i("CONN_TASK connection");
        }

        break;

    case CONN_RECONNECTION:
        if (CONN_taskData.oldfunction != CONN_taskData.function)
        {
            CONN_taskData.oldfunction = CONN_taskData.function;
            CONN_taskData.tickStart = tickNow;
            log_i("CONN_TASK reconnection");
        }
        break;

    default:
        CONN_taskInit();
    }

    // if (WIFI_taskData.statusFlags & BIT_INIT_OK)
    // {
    //     if (WIFI_taskData.function < WIFI_SCAN)
    //     {
    //         static uint8_t cnt = 0;
    //         cnt++;
    //         lv_label_set_text_fmt(ui_lblCountValue, "%d", cnt);
    //     }
    // }
}