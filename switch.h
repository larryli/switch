#ifndef _SWITCH_H
#define _SWITCH_H

#include <ESP8266WiFi.h> // @see https://github.com/esp8266/Arduino
#include <ESPAsyncWebServer.h> // @see https://github.com/me-no-dev/ESPAsyncWebServer

#define SWITCH_COUNT 2              // 继电器数量
#define SWITCH_TRIG_LOW             // 继电器低电平触发
#define SWITCH_IR                   // 开启红外
#define SWITCH_OLED                 // 开启显示屏
#define SWITCH_DEBUG                // 开启 TX0 调试输出
#define SWITCH_NAME "SWITCH_"       // mDNS 名称前缀
#define SWITCH_SERV "http_switch"   // mDNS 服务名

#ifdef SWITCH_DEBUG
# ifdef SWITCH_IR
#  ifdef SWITCH_OLED
#   if SWITCH_COUNT > 5
#    define SWITCH_COUNT 5
#   endif
static const uint8_t SWITCHES[] = {D1, D2, D0, D8, D9};
#  else
#   if SWITCH_COUNT > 7
#    define SWITCH_COUNT 7
#   endif
static const uint8_t SWITCHES[] = {D1, D2, D6, D7, D0, D8, D9};
#  endif
# else
#  ifdef SWITCH_OLED
#   if SWITCH_COUNT > 6
#    define SWITCH_COUNT 6
#   endif
static const uint8_t SWITCHES[] = {D1, D2, D5, D0, D8, D9};
#  else
#   if SWITCH_COUNT > 8
#    define SWITCH_COUNT 8
#   endif
static const uint8_t SWITCHES[] = {D1, D2, D5, D6, D7, D0, D8, D9};
#  endif
# endif

# define debug_setup()  Serial.begin(115200);  \
Serial.println()
# define debug_print(...)    Serial.print(__VA_ARGS__)
# define debug_println(...)  Serial.println(__VA_ARGS__)
#else
# ifdef SWITCH_IR
#  ifdef SWITCH_OLED
#   if SWITCH_COUNT > 6
#    define SWITCH_COUNT 6
#   endif
static const uint8_t SWITCHES[] = {D1, D2, D0, D8, D9, D10};
#  else
#   if SWITCH_COUNT > 8
#    define SWITCH_COUNT 8
#   endif
static const uint8_t SWITCHES[] = {D1, D2, D6, D7, D0, D8, D9, D10};
#  endif
# else
#  ifdef SWITCH_OLED
#   if SWITCH_COUNT > 7
#    define SWITCH_COUNT 7
#   endif
static const uint8_t SWITCHES[] = {D1, D2, D5, D0, D8, D9, D10};
#  else
#   if SWITCH_COUNT > 9
#    define SWITCH_COUNT 9
#   endif
static const uint8_t SWITCHES[] = {D1, D2, D5, D6, D7, D0, D8, D9, D10};
#  endif
# endif

# define debug_setup()
# define debug_print(...)
# define debug_println(...)
#endif

#ifdef SWITCH_TRIG_LOW
# define SWITCH_ON LOW
# define SWITCH_OFF HIGH
#else
# define SWITCH_ON HIGH
# define SWITCH_OFF LOW
#endif

#define WIFI_CONNECTING 0
#define WIFI_DISCONNECTED 1
#define WIFI_CONFIG 2
#define WIFI_CONNECTED 3

static const uint8_t WIFI_LED = D4;
static const uint8_t RESET_BTN = D3;

#ifdef SWITCH_IR
static const uint8_t IR_RECV = D5;
#endif

#ifdef SWITCH_OLED
static const uint8_t OLED_SDA = D6;
static const uint8_t OLED_SCL = D7;
#endif

// Switch
void switch_setup();
void switch_reset();
bool switch_toggle(unsigned int i);
bool switch_turn(unsigned int i, bool state);

// Led
void led_setup();
void led_connect();
void led_connected();
void led_disconnected();
void led_config();
void led_reset();
void led_switch();

// Reset
void reset_setup();

#ifdef SWITCH_IR
// IR Recv
void irrecv_setup();
bool irrecv_loop();
#endif

#ifdef SWITCH_OLED
// OLED
void oled_setup();
void oled_loop();
void oled_up();
void oled_down();
void oled_select();
void oled_refresh();
void oled_qrcode();
void oled_clear();
#else
# define oled_refresh()
#endif

// Wifi
void wifi_setup();
bool wifi_loop();
bool wifi_is_connected();
uint8_t wifi_get_state();

// Server
void server_setup();
void server_loop();
void server_start();
void server_stop();
void server_update();

// mDNS
void mdns_setup();
bool mdns_start();

#endif
