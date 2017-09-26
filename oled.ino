#include "switch.h"
#include <U8g2lib.h> // @see https://github.com/olikraus/u8g2
#include <qrcode.h> // @see https://github.com/ricmoo/QRCode

#ifdef SWITCH_OLED

#include "oled_font.h"

#define OLED_HOME 0
#define OLED_SETTING 1
#define OLED_CONFIRM 2

static U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, OLED_SCL, OLED_SDA);
static bool refresh = true;
static uint8_t state = OLED_HOME;

static const char TITLE[] = "智 能 开 关";
static const char SETTING[] = " 设置 ";
static const char* NAMES[] = {"⒈", "⒉", "⒊", "⒋", "⒌", "⒍", "⒎"};
static String http = String("");
static QRCode qrcode;
static uint8_t qrcodeData[56];  // (21 * 21 + 7) / 8

void oled_setup()
{
  u8g2.begin();
}

void oled_loop()
{
  if (!refresh) {
    return;
  }
  u8g2.firstPage();
  do {
    int x, w;

    u8g2.setFont(oled_font);
    u8g2.setDrawColor(1);
    w = u8g2.getUTF8Width(TITLE);
    x = (74 - w) / 2 - 1;
    u8g2.drawUTF8(x, 11, TITLE);
    oled_switches();
#ifdef SWITCH_IR
    w = u8g2.getUTF8Width(SETTING);
    x = (74 - w) / 2 - 1;
    u8g2.drawBox(x - 1, 39, w + 1, 13);
    u8g2.setDrawColor(0);
    u8g2.drawUTF8(x, 50, SETTING);
    u8g2.setDrawColor(1);
#endif
    u8g2.drawBox(73, 0, 54, 54);
    u8g2.setDrawColor(0);
    switch (wifi_get_state()) {
      case WIFI_CONNECTING:
        u8g2.drawUTF8(88, 26, "正在");
        u8g2.drawUTF8(88, 38, "连网");
        u8g2.setFont(u8g2_font_5x7_tr);
        u8g2.setDrawColor(1);
        u8g2.drawStr(0, 63, (String("SSID: ") + WiFi.SSID()).c_str());
        break;
      case WIFI_CONNECTED:
        for (uint8_t y = 0; y < qrcode.size; y++) {
          for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
              u8g2.drawBox(2 * x + 79, 2 * y + 6, 2, 2);
            }
          }
        }
        u8g2.setDrawColor(1);
        u8g2.setFont(u8g2_font_5x7_tr);
        w = u8g2.getUTF8Width(http.c_str());
        u8g2.drawUTF8(127 - w, 63, http.c_str());
        break;
      case WIFI_DISCONNECTED:
        u8g2.drawUTF8(88, 26, "正在");
        u8g2.drawUTF8(88, 38, "重连");
        u8g2.setFont(u8g2_font_5x7_tr);
        u8g2.drawStr(0, 63, (String("SSID: ") + WiFi.SSID()).c_str());
        break;
      case WIFI_CONFIG:
        u8g2.drawUTF8(88, 26, "正在");
        u8g2.drawUTF8(88, 38, "配网");
        break;
    }
  } while (u8g2.nextPage());
  refresh = false;
}

void oled_up()
{
}

void oled_down()
{
}

void oled_select()
{
}

void oled_refresh()
{
  if (state == OLED_HOME) {
    refresh = true;
  }
}

void oled_qrcode()
{
  http = String("HTTP://") + WiFi.localIP().toString() + String("/");
  qrcode_initText(&qrcode, qrcodeData, 1, 0, http.c_str());
  http.toLowerCase();
}

void oled_clear()
{
  u8g2.firstPage();
  do {
    u8g2.clear();
  } while (u8g2.nextPage());
  refresh = false;
}

#ifdef SWITCH_IR
# define Y_1_1 31
# define Y_2_1 24
# define Y_2_2 37
#else
# define Y_1_1 37
# define Y_2_1 30
# define Y_2_2 43
#endif

static void oled_switches()
{
#if SWITCH_COUNT == 1
  oled_switch(24, Y_1_1, 0);
#elif SWITCH_COUNT == 2
  oled_switch(8, Y_1_1, 0);
  oled_switch(40, Y_1_1, 1);
#elif SWITCH_COUNT == 3
  oled_switch(1, Y_1_1, 0);
  oled_switch(25, Y_1_1, 1);
  oled_switch(49, Y_1_1, 2);
#elif SWITCH_COUNT == 4
  oled_switch(8, Y_2_1, 0);
  oled_switch(40, Y_2_1, 1);
  oled_switch(8, Y_2_2, 2);
  oled_switch(40, Y_2_2, 3);
#elif SWITCH_COUNT == 5
  oled_switch(1, Y_2_1, 0);
  oled_switch(25, Y_2_1, 1);
  oled_switch(49, Y_2_1, 2);
  oled_switch(8, Y_2_2, 3);
  oled_switch(40, Y_2_2, 4);
#elif SWITCH_COUNT == 6
  oled_switch(1, Y_2_1, 0);
  oled_switch(25, Y_2_1, 1);
  oled_switch(49, Y_2_1, 2);
  oled_switch(1, Y_2_2, 3);
  oled_switch(25, Y_2_2, 4);
  oled_switch(49, Y_2_2, 5);
#elif SWITCH_COUNT == 7
  oled_switch(1, 24, 0);
  oled_switch(25, 24, 1);
  oled_switch(49, 24, 2);
  oled_switch(8, 37, 3);
  oled_switch(40, 37, 4);
  oled_switch(8, 50, 5);
  oled_switch(40, 50, 6);
#endif
}

static void oled_switch(int x, int y, int i)
{
  u8g2.drawUTF8(x, y, NAMES[i]);
  u8g2.drawUTF8(x + 12, y, digitalRead(SWITCHES[i]) == SWITCH_ON ? "●" : "○");
}

#endif
