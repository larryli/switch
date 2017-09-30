///
// 重置配网
//

#include "switch.h"
#include <Ticker.h>

#define RESET_HOLD_MS 3000
static unsigned long _reset_last;
static Ticker _reset_ticker;

///
// 重置按键配置
//
void reset_setup()
{
  pinMode(RESET_BTN, INPUT);
  _reset_last = millis();
  attachInterrupt(digitalPinToInterrupt(RESET_BTN), _reset_change, CHANGE);
}

void reset_event(const Event e)
{
  if (e != EVENT_RESET) {
    return;
  }
  // 系统重置，清除 Wifi 配置，重启
  debug_print(F("[DEBUG] RESET: "));
  WiFi.begin("");
  WiFi.disconnect();
  ESP.eraseConfig();
  _reset_ticker.once_ms(1000, [] {
    debug_println(F("DONE!"));
    ESP.restart();
  });
}

///
// 系统重置，清除 Wifi 配置，重启
//
static void _reset_change()
{
  bool push = digitalRead(RESET_BTN) == LOW;
  unsigned long time = millis();

  debug_print(F("[DEBUG] Reset: "));
  if (push) {
    debug_println(F("push button"));
    _reset_last = time;
  } else {
    debug_println(F("release button"));
    if (time - _reset_last > RESET_HOLD_MS) {
      // This number will overflow (go back to zero), after approximately 50 days.
      debug_println(F("[DEBUG] Reset: hold"));
      switch_event(EVENT_RESET);
    }
  }
}
