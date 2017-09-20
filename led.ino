///
// LED
//

#include "switch.h"
#include <Ticker.h>

static int led_tick;
static Ticker led_ticker;

///
// LED 配置
//
void led_setup()
{
  pinMode(WIFI_LED, OUTPUT);
  digitalWrite(WIFI_LED, LOW);
}

///
// LED 正在联网，闪两下，亮 0.5s
//
void led_connect()
{
  led_tick = 0;
  digitalWrite(WIFI_LED, HIGH);
  led_ticker.attach(0.1, led_flip_connect);
}

///
// LED 联网正常，常亮
//
void led_connected()
{
  led_ticker.detach();
  digitalWrite(WIFI_LED, LOW);
}

///
// LED 联网错误，慢闪，亮 0.5 秒，灭 1.5 秒
//
void led_disconnected()
{
  led_tick = 0;
  digitalWrite(WIFI_LED, LOW);
  led_ticker.attach(0.5, led_flip_disconnected);
}

///
// LED 配网状态，快闪
//
void led_config()
{
  digitalWrite(WIFI_LED, HIGH);
  led_ticker.attach(0.3, led_flip);
}

///
// LED 清除配置重启，超快闪
//
void led_reset()
{
  led_ticker.attach(0.2, led_flip);
}

///
// LED 开关动作，只闪一次，只在网络正常时使用
//
void led_switch()
{
  if (wifi_is_connected()) {
    digitalWrite(WIFI_LED, HIGH);
    led_ticker.attach(0.1, led_flip_switch);
  }
}

static void led_flip_connect()
{
  ++led_tick;
  if (led_tick == 1 || led_tick == 3 || led_tick == 5) {
    digitalWrite(WIFI_LED, LOW);
  } else if (led_tick == 2 || led_tick == 4) {
    digitalWrite(WIFI_LED, HIGH);
  } else if (led_tick >= 10) {
    digitalWrite(WIFI_LED, HIGH);
    led_tick = 0;
  }
}

static void led_flip_disconnected()
{
  ++led_tick;
  if (led_tick == 1) {
    digitalWrite(WIFI_LED, HIGH);
  } else if (led_tick >= 4) {
    digitalWrite(WIFI_LED, LOW);
    led_tick = 0;
  }
}

static void led_flip_switch()
{
  digitalWrite(WIFI_LED, LOW);
  led_ticker.detach();
}

static void led_flip()
{
  int state = digitalRead(WIFI_LED);

  digitalWrite(WIFI_LED, !state);
}
