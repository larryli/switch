///
// LED
//

#include "switch.h"
#include <Ticker.h>

static int _led_tick;
static Ticker _led_ticker;
static bool _led_state;  // 网络是否正常

///
// LED 配置
//
void led_setup()
{
  pinMode(WIFI_LED, OUTPUT);
  digitalWrite(WIFI_LED, LOW);
  _led_state = false;
}

void led_event(const Event e)
{
  switch (e) {
    case EVENT_CONNECTING: // 正在联网，闪两下，亮 0.5s
      _led_state = false;
      _led_tick = 0;
      digitalWrite(WIFI_LED, HIGH);
      _led_ticker.attach_ms(100, _led_connect);
      return;
    case EVENT_CONNECTED: // 联网正常，常亮
      _led_state = true;
      _led_ticker.detach();
      digitalWrite(WIFI_LED, LOW);
      return;
    case EVENT_DISCONNECTED: // 联网错误，慢闪，亮 0.5 秒，灭 1.5 秒
      _led_state = false;
      _led_tick = 0;
      digitalWrite(WIFI_LED, LOW);
      _led_ticker.attach_ms(500, _led_disconnected);
      return;
    case EVENT_CONFIG: // 配网状态，快闪
      _led_state = false;
      digitalWrite(WIFI_LED, HIGH);
      _led_ticker.attach_ms(300, _led_flip);
      return;
    case EVENT_RESET: // 清除配置重启，超快闪
      _led_state = false;
      _led_ticker.attach_ms(200, _led_flip);
      return;
    case EVENT_REFRESH: // 开关动作，只闪一次，只在网络正常时使用
      if (_led_state) {
        digitalWrite(WIFI_LED, HIGH);
        _led_ticker.once_ms(100, _led_refresh);
      }
      return;
  }
}

static void _led_connect()
{
  ++_led_tick;
  if (_led_tick == 1 || _led_tick == 3 || _led_tick == 5) {
    digitalWrite(WIFI_LED, LOW);
  } else if (_led_tick == 2 || _led_tick == 4) {
    digitalWrite(WIFI_LED, HIGH);
  } else if (_led_tick >= 10) {
    digitalWrite(WIFI_LED, HIGH);
    _led_tick = 0;
  }
}

static void _led_disconnected()
{
  ++_led_tick;
  if (_led_tick == 1) {
    digitalWrite(WIFI_LED, HIGH);
  } else if (_led_tick >= 4) {
    digitalWrite(WIFI_LED, LOW);
    _led_tick = 0;
  }
}

static void _led_refresh()
{
  digitalWrite(WIFI_LED, LOW);
}

static void _led_flip()
{
  int state = digitalRead(WIFI_LED);

  digitalWrite(WIFI_LED, !state);
}
