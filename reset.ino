#include "switch.h"

///
// 重置按键配置
//
void reset_setup()
{
  pinMode(RESET_BTN, INPUT);
  attachInterrupt(digitalPinToInterrupt(RESET_BTN), reset_handle, FALLING);
}

///
// 系统重置，清除 Wifi 配置，重启
//
void reset_handle()
{
  debug_println(F("[DEBUG] RESET: "));
  led_reset();
  WiFi.begin("");
  WiFi.disconnect();
  ESP.eraseConfig();
  delay(100);
  ESP.restart();
}
