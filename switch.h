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

static const uint8_t WIFI_LED = D4;
static const uint8_t RESET_BTN = D3;

#ifdef SWITCH_IR
static const uint8_t IR_RECV = D5;
#endif

#ifdef SWITCH_OLED
static const uint8_t OLED_SDA = D6;
static const uint8_t OLED_SCL = D7;
#endif

typedef enum {
  EVENT_CONNECTING,
  EVENT_CONNECTED,
  EVENT_FAILED,
  EVENT_DISCONNECTED,
  EVENT_CONFIG,
  EVENT_RECEIVED,
  EVENT_RESET,
  EVENT_REFRESH,
  EVENT_ON,
  EVENT_OFF,
  EVENT_1,
  EVENT_2,
  EVENT_3,
  EVENT_4,
  EVENT_5,
  EVENT_6,
  EVENT_7,
  EVENT_8,
  EVENT_9,
  EVENT_1_ON,
  EVENT_2_ON,
  EVENT_3_ON,
  EVENT_4_ON,
  EVENT_5_ON,
  EVENT_6_ON,
  EVENT_7_ON,
  EVENT_8_ON,
  EVENT_9_ON,
  EVENT_1_OFF,
  EVENT_2_OFF,
  EVENT_3_OFF,
  EVENT_4_OFF,
  EVENT_5_OFF,
  EVENT_6_OFF,
  EVENT_7_OFF,
  EVENT_8_OFF,
  EVENT_9_OFF,
  EVENT_UP,
  EVENT_DOWN,
  EVENT_SELECT,
} Event;

// Switch
void switch_event(const Event e);

// Led
void led_setup();
void led_event(const Event e);

// Reset
void reset_setup();
void reset_event(const Event e);

#ifdef SWITCH_IR
// IR Recv
void irrecv_setup();
bool irrecv_loop();
#endif

#ifdef SWITCH_OLED
// OLED
void oled_setup();
void oled_loop();
void oled_event(const Event e);
#endif

// Wifi
void wifi_setup();
bool wifi_loop();
void wifi_event(const Event e);

// Server
void server_setup();
void server_event(const Event e);

// mDNS
void mdns_setup();
void mdns_event(const Event e);

#endif
