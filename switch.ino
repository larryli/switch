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
  switch_setup();
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
// 配置开关
//
void switch_setup()
{
  for (int i = 0; i < SWITCH_COUNT; i++) {
    pinMode(SWITCHES[i], OUTPUT);
    digitalWrite(SWITCHES[i], SWITCH_OFF);
  }
}

///
// 切换开关状态
// @param unsigned int i 开关索引
// @return bool 开关是否已打开
//
bool switch_toggle(unsigned int i)
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
bool switch_turn(unsigned int i, bool state)
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
