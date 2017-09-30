///
// switch.h
// 去掉注释可以设置相应选项
//

#ifndef _SWITCH_H
#define _SWITCH_H

//#define SWITCH_COUNT 9             // 继电器数量
//#define SWITCH_TRIG_HIGH           // 继电器高电平触发
//#define SWITCH_NO_IR               // 关闭红外遥控
//#define SWITCH_NO_OLED             // 关闭显示屏
//#define SWITCH_NO_DEBUG            // 关闭调试输出
//#define SWITCH_NAME "SWITCH_"      // mDNS 名称前缀
//#define SWITCH_SERV "http_switch"  // mDNS 服务名

#include <ESP8266WiFi.h> // @see https://github.com/esp8266/Arduino
#include <ESPAsyncWebServer.h> // @see https://github.com/me-no-dev/ESPAsyncWebServer
#include <Ticker.h>

#ifndef SWITCH_NAME
# define SWITCH_NAME "SWITCH_"       // mDNS 名称前缀
#endif
#ifndef SWITCH_SERV
# define SWITCH_SERV "http_switch"   // mDNS 服务名
#endif

#ifndef SWITCH_TRIG_LOW
# ifndef SWITCH_TRIG_HIGH
#  define SWITCH_TRIG_LOW            // 继电器低电平触发
# endif
#endif
#ifdef SWITCH_TRIG_LOW
# define _D_LOW_
# define _N_LOW_ 0
# define SWITCH_ON LOW
# define SWITCH_OFF HIGH
#else
# define _D_LOW_ D8,
# define _N_LOW_ 1
# define SWITCH_ON HIGH
# define SWITCH_OFF LOW
#endif

const uint8_t WIFI_LED = D4;
const uint8_t RESET_BTN = D3;

#ifndef SWITCH_IR
# ifndef SWITCH_NO_IR
#  define SWITCH_IR                  // 开启红外
# endif
#endif
#ifdef SWITCH_IR
# define _D_IR_
# define _N_IR_ 0
const uint8_t IR_RECV = D5;
#else
# define _D_IR_ D5,
# define _N_IR_ 1
#endif

#ifndef SWITCH_OLED
# ifndef SWITCH_NO_OLED
#  define SWITCH_OLED                // 开启显示屏
# endif
#endif
#ifdef SWITCH_OLED
# define _D_OLED_
# define _N_OLED_ 0
const uint8_t OLED_SDA = D6;
const uint8_t OLED_SCL = D7;
#else
# define _D_OLED_ D6, D7,
# define _N_OLED_ 2
#endif

#ifndef SWITCH_DEBUG
# ifndef SWITCH_NO_DEBUG
#  define SWITCH_DEBUG               // 开启 TX0 调试输出
# endif
#endif
#ifdef SWITCH_DEBUG
# define _D_DEBUG_
# define _N_DEBUG_ 0
# define debug_setup() Serial.begin(115200);  \
Serial.println()
# define debug_print(...) Serial.print(__VA_ARGS__)
# define debug_println(...) Serial.println(__VA_ARGS__)
#else
# define _D_DEBUG_ D10,
# define _N_DEBUG_ 1
# define debug_setup() uart_set_debug(0)
# define debug_print(...)
# define debug_println(...)
#endif

#ifndef SWITCH_COUNT
# define SWITCH_COUNT 2              // 继电器数量
#endif
#define _N_COUNT_ (4+_N_IR_+_N_OLED_+_N_LOW_+_N_DEBUG_)
#if SWITCH_COUNT > _N_COUNT_
# define SWITCH_COUNT _N_COUNT_
#endif
const uint8_t SWITCHES[] = {D1, D2, _D_IR_ _D_OLED_ D0, D9, _D_LOW_ _D_DEBUG_};
#undef _D_IR_
#undef _D_OLED_
#undef _D_LOW_
#undef _D_DEBUG_

typedef enum {
  EVENT_CONNECTING, EVENT_CONNECTED, EVENT_FAILED, EVENT_DISCONNECTED,
  EVENT_CONFIG, EVENT_RECEIVED,
  EVENT_RESET, EVENT_RESTART,
  EVENT_REFRESH,
  EVENT_ON, EVENT_OFF,
  EVENT_1, EVENT_2, EVENT_3, EVENT_4, EVENT_5, EVENT_6, EVENT_7, EVENT_8, EVENT_9,
  EVENT_1_ON, EVENT_2_ON, EVENT_3_ON, EVENT_4_ON, EVENT_5_ON, EVENT_6_ON, EVENT_7_ON, EVENT_8_ON, EVENT_9_ON,
  EVENT_1_OFF, EVENT_2_OFF, EVENT_3_OFF, EVENT_4_OFF, EVENT_5_OFF, EVENT_6_OFF, EVENT_7_OFF, EVENT_8_OFF, EVENT_9_OFF,
  EVENT_UP, EVENT_DOWN, EVENT_SELECT,
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
