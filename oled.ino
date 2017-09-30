///
// OLED 显示
//

#ifdef SWITCH_OLED

#include "switch.h"
#include <U8g2lib.h> // @see https://github.com/olikraus/u8g2
#include <qrcode.h> // @see https://github.com/ricmoo/QRCode
#include "oled_font.h"

typedef enum {
  OLED_HOME,
  OLED_RESET,
  OLED_SETTING,
  OLED_CONFIRM,
  OLED_CLOSE,
} OledState;

typedef enum {
  OLED_CONNECTING,
  OLED_CONNECTED,
  OLED_RECONNECTING,
  OLED_CONFIG,
} OledWifi;

static U8G2_SSD1306_128X64_NONAME_1_SW_I2C _u8g2(U8G2_R0, OLED_SCL, OLED_SDA);
static bool _oled_refresh;
static OledState _oled_state;
static OledWifi _oled_wifi;

static const char _oled_str_title[] U8X8_PROGMEM = "智 能 开 关";
static const char _oled_str_setting[] U8X8_PROGMEM = " 设 置 ";
static const char _oled_str_reset[] U8X8_PROGMEM = " 重 置 配 网 ";
static const char _oled_str_now[] U8X8_PROGMEM = "正 在";
static const char _oled_str_connecting[] U8X8_PROGMEM = "连 网";
static const char _oled_str_reconnecting[] U8X8_PROGMEM = "重 连";
static const char _oled_str_config[] U8X8_PROGMEM = "配 网";
static const char _oled_str_ssid[] U8X8_PROGMEM = "SSID: ";
static const char* _oled_str_switches[] U8X8_PROGMEM = {"⒈", "⒉", "⒊", "⒋", "⒌", "⒍", "⒎"};
static const char* _oled_str_states[] U8X8_PROGMEM = {"●", "○"};
#define OLED_MENU_BACK 0
#define OLED_MENU_CLOSE 1
#define OLED_MENU_CONFIG 2
#define OLED_MENU_RESTART 3
#define OLED_MENU_RESET 4
#define OLED_MENU_COUNT 5
static const char* _oled_str_menus[] U8X8_PROGMEM = {"返回主界面", "关闭屏幕", "配置网络", "重新启动", "重置配网"};
static const char* _oled_str_tips[] U8X8_PROGMEM = {"您确定要", "么？"};
static const char* _oled_str_confirms[] U8X8_PROGMEM = {" 确 定 ", " 取 消 "};
static String _oled_http = String("");
static int _oled_select;
static bool _oled_confirm;
static char *_oled_tips;
static QRCode _oled_qrcode;
static uint8_t _oled_qrcodeData[56];  // (21 * 21 + 7) / 8
#define OLED_SAVER 30
static Ticker _oled_ticker;

void oled_setup()
{
  _u8g2.begin();
  _oled_refresh = true;
  _oled_state = OLED_HOME;
  _oled_wifi = OLED_CONNECTING;
  _oled_saver();
}

void oled_loop()
{
  if (!_oled_refresh) {
    return;
  }
  _u8g2.firstPage();
  do {
    switch (_oled_state) {
      case OLED_HOME:
        _oled_home0();
        switch (_oled_wifi) {
          case OLED_CONNECTED:
            _oled_home1();
            break;
          case OLED_CONNECTING:
            _oled_home2(_oled_str_connecting, true);
            break;
          case OLED_RECONNECTING:
            _oled_home2(_oled_str_reconnecting, true);
            break;
          case OLED_CONFIG:
            _oled_home2(_oled_str_config, false);
            break;
        }
        break;
      case OLED_RESET:
        _oled_reset();
        break;
      case OLED_SETTING:
        _oled_setting();
        break;
      case OLED_CONFIRM:
        _oled_confirm0();
        break;
      case OLED_CLOSE:
        _u8g2.clearDisplay();
        break;
    }
  } while (_u8g2.nextPage());
  _oled_refresh = false;
}

void oled_event(const Event e)
{
  switch (e) {
    case EVENT_CONNECTING: // 正在连网
      _oled_wifi = OLED_CONNECTING;
      break;
    case EVENT_CONNECTED: // 连网成功
      _oled_http = String(F("HTTP://")) + WiFi.localIP().toString() + String(F("/"));
      qrcode_initText(&_oled_qrcode, _oled_qrcodeData, 1, 0, _oled_http.c_str());
      _oled_http.toLowerCase();
      _oled_wifi = OLED_CONNECTED;
      break;
    case EVENT_DISCONNECTED: // 掉线重连
      _oled_wifi = OLED_RECONNECTING;
      break;
    case EVENT_CONFIG: // 配网
      _oled_wifi = OLED_CONFIG;
      break;
    case EVENT_RESET: // 重置
      _oled_state = OLED_RESET;
      break;
    case EVENT_REFRESH: // 刷新显示
      if (_oled_state == OLED_CLOSE) {
        return;
      }
      break;
    case EVENT_UP:
      switch (_oled_state) {
        case OLED_SETTING:
          if (_oled_select > 0) {
            _oled_select--;
          }
          break;
        case OLED_CONFIRM:
          _oled_confirm = !_oled_confirm;
          break;
        default:
          switch_event(EVENT_1);
          break;
      }
      break;
    case EVENT_DOWN:
      switch (_oled_state) {
        case OLED_SETTING:
          if (_oled_select < OLED_MENU_COUNT - 1) {
            _oled_select++;
          }
          break;
        case OLED_CONFIRM:
          _oled_confirm = !_oled_confirm;
          break;
        default:
#ifdef SWITCH_COUNT > 1
          switch_event(EVENT_2);
#endif
          break;
      }
      break;
    case EVENT_SELECT:
      switch (_oled_state) {
        case OLED_SETTING:
          if (_oled_select == OLED_MENU_BACK) {
            _oled_state = OLED_HOME;
          } else {
            _oled_state = OLED_CONFIRM;
            _oled_confirm = false;
          }
          break;
        case OLED_CONFIRM:
          if (_oled_confirm) {
            switch (_oled_select) {
              case OLED_MENU_CLOSE:
                _oled_state = OLED_CLOSE;
                break;
              case OLED_MENU_CONFIG:
                switch_event(EVENT_CONFIG);
                _oled_state = OLED_HOME;
                break;
              case OLED_MENU_RESTART:
                switch_event(EVENT_RESTART);
                break;
              case OLED_MENU_RESET:
                switch_event(EVENT_RESET);
                break;
            }
          } else {
            _oled_state = OLED_SETTING;
          }
          break;
        case OLED_CLOSE:
          _oled_state = OLED_HOME;
          break;
        default:
          _oled_state = OLED_SETTING;
          _oled_select = 0;
          break;
      }
      break;
    default:
      return;
  }
  if (_oled_state == OLED_CLOSE) {
    _oled_ticker.detach();
  } else {
    _oled_saver();
  }
  _oled_refresh = true;
}

static void _oled_saver()
{
  _oled_ticker.detach();
  _oled_ticker.once(OLED_SAVER, [] {
    _oled_state = OLED_CLOSE;
    _oled_refresh = true;
  });
}

static void _oled_reset()
{
  int x, w;

  _u8g2.setFont(oled_font);
  _u8g2.setDrawColor(1);
  w = _u8g2.getUTF8Width(_oled_str_reset);
  x = (128 - w) / 2;
  _u8g2.drawBox(x, 26, w, 13);
  _u8g2.setDrawColor(0);
  _u8g2.drawUTF8(x, 37, _oled_str_reset);
}

static void _oled_home0()
{
  int x, w;

  _u8g2.setFont(oled_font);
  _u8g2.setDrawColor(1);
  w = _u8g2.getUTF8Width(_oled_str_title);
  x = (74 - w) / 2;
  _u8g2.drawUTF8(x, 11, _oled_str_title);
  _oled_switches();
#ifdef SWITCH_IR
  w = _u8g2.getUTF8Width(_oled_str_setting);
  x = (74 - w) / 2;
  _u8g2.drawBox(x, 39, w, 13);
  _u8g2.setDrawColor(0);
  _u8g2.drawUTF8(x, 50, _oled_str_setting);
  _u8g2.setDrawColor(1);
#endif
  _u8g2.drawBox(73, 0, 54, 54);
  _u8g2.setDrawColor(0);
}

static void _oled_home1()
{
  int w;

  for (uint8_t y = 0; y < _oled_qrcode.size; y++) {
    for (uint8_t x = 0; x < _oled_qrcode.size; x++) {
      if (qrcode_getModule(&_oled_qrcode, x, y)) {
        _u8g2.drawBox(2 * x + 79, 2 * y + 6, 2, 2);
      }
    }
  }
  _u8g2.setDrawColor(1);
  _u8g2.setFont(u8g2_font_5x7_tr);
  w = _u8g2.getUTF8Width(_oled_http.c_str());
  _u8g2.drawUTF8(127 - w, 63, _oled_http.c_str());
}

static void _oled_home2(const char *msg, const bool show)
{
  int x, w;

  w = _u8g2.getUTF8Width(_oled_str_now);
  x = (54 - w) / 2 + 74;
  _u8g2.drawUTF8(x, 24, _oled_str_now);
  w = _u8g2.getUTF8Width(msg);
  x = (54 - w) / 2 + 74;
  _u8g2.drawUTF8(x, 40, msg);
  if (show) {
    _u8g2.setFont(u8g2_font_5x7_tr);
    _u8g2.setDrawColor(1);
    _u8g2.drawStr(0, 63, (String(_oled_str_ssid) + WiFi.SSID()).c_str());
  }
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

static void _oled_switches()
{
#if SWITCH_COUNT == 1
  _oled_switch(24, Y_1_1, 0);
#elif SWITCH_COUNT == 2
  _oled_switch(8, Y_1_1, 0);
  _oled_switch(40, Y_1_1, 1);
#elif SWITCH_COUNT == 3
  _oled_switch(1, Y_1_1, 0);
  _oled_switch(25, Y_1_1, 1);
  _oled_switch(49, Y_1_1, 2);
#elif SWITCH_COUNT == 4
  _oled_switch(8, Y_2_1, 0);
  _oled_switch(40, Y_2_1, 1);
  _oled_switch(8, Y_2_2, 2);
  _oled_switch(40, Y_2_2, 3);
#elif SWITCH_COUNT == 5
  _oled_switch(1, Y_2_1, 0);
  _oled_switch(25, Y_2_1, 1);
  _oled_switch(49, Y_2_1, 2);
  _oled_switch(8, Y_2_2, 3);
  _oled_switch(40, Y_2_2, 4);
#elif SWITCH_COUNT == 6
  _oled_switch(1, Y_2_1, 0);
  _oled_switch(25, Y_2_1, 1);
  _oled_switch(49, Y_2_1, 2);
  _oled_switch(1, Y_2_2, 3);
  _oled_switch(25, Y_2_2, 4);
  _oled_switch(49, Y_2_2, 5);
#elif SWITCH_COUNT == 7
  _oled_switch(1, 24, 0);
  _oled_switch(25, 24, 1);
  _oled_switch(49, 24, 2);
  _oled_switch(8, 37, 3);
  _oled_switch(40, 37, 4);
  _oled_switch(8, 50, 5);
  _oled_switch(40, 50, 6);
#endif
}

static void _oled_switch(int x, int y, int i)
{
  _u8g2.drawUTF8(x, y, _oled_str_switches[i]);
  _u8g2.drawUTF8(x + 12, y, digitalRead(SWITCHES[i]) == SWITCH_ON ? _oled_str_states[0] : _oled_str_states[1]);
}

static void _oled_setting()
{
  _u8g2.setFont(oled_font);
  for (int i = 0; i < OLED_MENU_COUNT; i++) {
    _u8g2.setDrawColor(1);
    if (i == _oled_select) {
      _u8g2.drawBox(0, i * 13, 128, 13);
      _u8g2.setDrawColor(0);
    }
    _u8g2.drawUTF8(3, i * 13 + 11, _oled_str_menus[i]);
  }
}

static void _oled_confirm0()
{
  int x, w;

  _u8g2.setFont(oled_font);
  _u8g2.setDrawColor(1);
  _u8g2.drawUTF8(3, 24, _oled_str_tips[0]);
  _u8g2.drawUTF8(51, 24, _oled_str_menus[_oled_select]);
  _u8g2.drawUTF8(99, 24, _oled_str_tips[1]);

  w = _u8g2.getUTF8Width(_oled_str_confirms[0]);
  x = (128 - 2 * w) / 3;

  if (_oled_confirm) {
    _u8g2.drawBox(x, 39, w, 13);
    _u8g2.setDrawColor(0);
    _u8g2.drawUTF8(x, 50, _oled_str_confirms[0]);
    _u8g2.setDrawColor(1);
    _u8g2.drawUTF8(x * 2 + w, 50, _oled_str_confirms[1]);
  } else {
    _u8g2.drawUTF8(x, 50, _oled_str_confirms[0]);
    _u8g2.drawBox(x * 2 + w, 39, w, 13);
    _u8g2.setDrawColor(0);
    _u8g2.drawUTF8(x * 2 + w, 50, _oled_str_confirms[1]);
  }
}
#endif
