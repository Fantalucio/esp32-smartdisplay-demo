
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sdkconfig.h"
#include <WiFi.h>

#include <Arduino.h>

#include <esp32_smartdisplay.h>
#include <ui/ui.h>

#include <src/extra/libs/qrcode/lv_qrcode.h>

#include "comm.h"


WIFI_taskData_t *wifiData;
CONN_taskData_t *connData;


/* void wifiScan()
{
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanComplete();
    if (n == WIFI_SCAN_FAILED)
    {
        Serial.println("Scan failed");
        WiFi.scanNetworks(true);
    }
    else if (n >= 0)
    {

        char text_buffer[32];

        String ssidString = WiFi.SSID(0);
        ssidString.toCharArray(text_buffer, ssidString.indexOf("\n"));
        lv_label_set_text(ui_lblWifiSsidValue, text_buffer);

        // Serial.printf("Found %d networks\n", n);
        for (int i = 0; i < 2; ++i)
        {
            sprintf(text_buffer, "%d: %s, RSSI: %d\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
            lv_list_add_btn(ui_wifiList, LV_SYMBOL_WIFI, text_buffer);
        }
        WiFi.scanNetworks(true);
    }
} */

void OnAddOneClicked(lv_event_t *e)
{
    static uint32_t cnt = 0;
    cnt++;
    lv_label_set_text_fmt(ui_lblCountValue, "%u", cnt);
}

void OnRotateClicked(lv_event_t *e)
{
    auto disp = lv_disp_get_default();
    auto rotation = (lv_display_rotation_t)((lv_disp_get_rotation(disp) + 1) % (LV_DISPLAY_ROTATION_270 + 1));
    lv_display_set_rotation(disp, rotation);
}

void setup()
{
#ifdef ARDUINO_USB_CDC_ON_BOOT
    delay(5000);
#endif

    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    //    WiFi.mode(WIFI_STA);
    //   WiFi.disconnect();

    wifiData = wifiTaskInit();
    connData = CONN_taskInit();

    Serial.begin(115200);
    Serial.setDebugOutput(true);
    log_i("Board: %s", BOARD_NAME);
    log_i("CPU: %s rev%d, CPU Freq: %d Mhz, %d core(s)", ESP.getChipModel(), ESP.getChipRevision(), getCpuFrequencyMhz(), ESP.getChipCores());
    log_i("Free heap: %d bytes", ESP.getFreeHeap());
    log_i("Free PSRAM: %d bytes", ESP.getPsramSize());
    log_i("SDK version: %s", ESP.getSdkVersion());

    smartdisplay_init();

    __attribute__((unused)) auto disp = lv_disp_get_default();
    // lv_disp_set_rotation(disp, LV_DISP_ROT_90);
    // lv_disp_set_rotation(disp, LV_DISP_ROT_180);
    // lv_disp_set_rotation(disp, LV_DISP_ROT_270);

    ui_init();

    // To use third party libraries, enable the define in lv_conf.h: #define LV_USE_QRCODE 1
    auto ui_qrcode = lv_qrcode_create(ui_scrMain);
    lv_qrcode_set_size(ui_qrcode, 100);
    lv_qrcode_set_dark_color(ui_qrcode, lv_color_black());
    lv_qrcode_set_light_color(ui_qrcode, lv_color_white());
    const char *qr_data = "https://github.com/rzeldent/esp32-smartdisplay";
    lv_obj_set_pos(ui_qrcode, 650, 240);
    //        lv_obj_set_x(ui_qrcode, 350);
    //    lv_obj_set_y(ui_qrcode, 300);
    lv_qrcode_update(ui_qrcode, qr_data, strlen(qr_data));
    //    lv_obj_center(ui_qrcode);
}

ulong next_millis;
auto lv_last_tick = millis();

void loop()
{

    manage_WIFI();
    manage_connection();

    auto const now = millis();
    if (now > next_millis)
    {
        next_millis = now + 1000;

        // wifiScan();

        // Wait a bit before scanning again

        char text_buffer[32];
        sprintf(text_buffer, "%lu", now);
        lv_label_set_text(ui_lblMillisecondsValue, text_buffer);

        /*
                String ssidString = WiFi.SSID(0);
                ssidString.toCharArray(text_buffer, ssidString.indexOf("\n"));
                lv_label_set_text(ui_lblWifiSsidValue,text_buffer);
        */

#ifdef BOARD_HAS_RGB_LED
        auto const rgb = (now / 2000) % 8;
        smartdisplay_led_set_rgb(rgb & 0x01, rgb & 0x02, rgb & 0x04);
#endif

#ifdef BOARD_HAS_CDS
        auto cdr = analogReadMilliVolts(CDS);
        sprintf(text_buffer, "%d", cdr);
        lv_label_set_text(ui_lblCdrValue, text_buffer);
#endif
    }

    // Update the ticker
    lv_tick_inc(now - lv_last_tick);
    lv_last_tick = now;
    // Update the UI
    lv_timer_handler();
}