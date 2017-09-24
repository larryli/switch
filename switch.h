#ifndef _SWITCH_H
#define _SWITCH_H

#include <ESP8266WiFi.h> // @see https://github.com/esp8266/Arduino

#define SWITCH_COUNT 2              // 继电器数量
#define SWITCH_TRIG_LOW             // 继电器低电平触发
#define SWITCH_DEBUG                // 开启 TX0 调试输出
#define SWITCH_NAME "SWITCH_"       // mDNS 名称前缀
#define SWITCH_SERV "http_switch"   // mDNS 服务名

static const uint8_t SWITCHES[] = {D1, D2, D6, D7, D0, D3, D9, D10};
static const uint8_t IR_RECV = D5;
static const uint8_t WIFI_LED = D4;
static const uint8_t RESET_BTN = D3;

#ifdef SWITCH_TRIG_LOW
#define SWITCH_ON LOW
#define SWITCH_OFF HIGH
#else
#define SWITCH_ON HIGH
#define SWITCH_OFF LOW
#endif

#ifdef SWITCH_DEBUG
#define debug_setup()  Serial.begin(115200);  \
  Serial.println()
#define debug_print(...)    Serial.print(__VA_ARGS__)
#define debug_println(...)  Serial.println(__VA_ARGS__)
#else
#define debug_setup()
#define debug_printRINT(...)
#define debug_println(...)
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

// IR Recv
void irrecv_setup();
bool irrecv_loop();

// Wifi
void wifi_setup();
bool wifi_loop();
bool wifi_is_connected();

// Server
void server_setup();
void server_loop();
void server_start();
void server_stop();

// mDNS
void mdns_setup();
bool mdns_start();

// Oled
void oled_setup();
void oled_loop();
void oled_up();
void oled_down();
void oled_select();


#endif
