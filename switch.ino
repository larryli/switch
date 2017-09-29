///
// 主程序，开关处理逻辑
//

#include "switch.h"

///
// 主配置入口
//
void setup()
{
  debug_setup();
  _switch_setup();
  led_setup();
  reset_setup();
#ifdef SWITCH_IR
  irrecv_setup();
#endif
#ifdef SWITCH_OLED
  oled_setup();
#endif
  mdns_setup();
  wifi_setup();
  server_setup();
}

///
// 主循环入口
//
void loop()
{
#ifdef SWITCH_OLED
  oled_loop();
#endif
#ifdef SWITCH_IR
  if (irrecv_loop()) {
    return;
  }
#endif
  if (wifi_loop()) {
    return;
  }
}

///
// 开关事件处理
//
void switch_event(const Event e)
{
  _switch_event(e);
  wifi_event(e);
  led_event(e);
#ifdef SWITCH_OLED
  oled_event(e);
#endif
  server_event(e);
  reset_event(e);
}

///
// 配置开关
//
static void _switch_setup()
{
  for (int i = 0; i < SWITCH_COUNT; i++) {
    pinMode(SWITCHES[i], OUTPUT);
    digitalWrite(SWITCHES[i], SWITCH_OFF);
  }
}

///
// 开关自身事件处理
//
static void _switch_event(const Event e)
{
  switch (e) {
    case EVENT_ON:
      for (int i = 0; i < SWITCH_COUNT; i++) {
        switch_turn(i, true);
      }
      break;
    case EVENT_OFF:
      for (int i = 0; i < SWITCH_COUNT; i++) {
        switch_turn(i, false);
      }
      break;
    default:
      if (e >= EVENT_1 && e <= EVENT_9) {
        switch_toggle(e - EVENT_1);
      } else if (e >= EVENT_1_ON && e <= EVENT_9_ON) {
        switch_turn(e - EVENT_1_ON, true);
      } else if (e >= EVENT_1_OFF && e <= EVENT_9_OFF) {
        switch_turn(e - EVENT_1_OFF, false);
      } else {
        return;
      }
      break;
  }
  switch_event(EVENT_REFRESH);
}

///
// 切换开关状态
// @param unsigned int i 开关索引
// @return bool 开关是否已打开
//
static bool switch_toggle(const uint8_t i)
{
  if (i < SWITCH_COUNT) {
    int data = !digitalRead(SWITCHES[i]);
    bool state = (data == SWITCH_ON);

    debug_print(F("[DEBUG] Switch #"));
    debug_print(i + 1);
    debug_println(state ? F(" on") : F(" off"));
    digitalWrite(SWITCHES[i], data);
    return state ? true : false;
  }
  return false;
}

///
// 打开/关闭开关
// @param unsigned int i 开关索引
// @param bool state 打开/关闭开关
// @return bool 是否操作成功
//
static bool switch_turn(const uint8_t i, const bool state)
{
  if (i < SWITCH_COUNT) {
    int data = digitalRead(SWITCHES[i]);

    if (state) {
      if (data == SWITCH_OFF) {
        debug_print(F("[DEBUG] Switch #"));
        debug_print(i + 1);
        debug_println(F(" on"));
        digitalWrite(SWITCHES[i], SWITCH_ON);
        return true;
      }
    } else if (data == SWITCH_ON) {
      debug_print(F("[DEBUG] Switch #"));
      debug_print(i + 1);
      debug_println(F(" off"));
      digitalWrite(SWITCHES[i], SWITCH_OFF);
      return true;
    }
  }
  return false;
}
