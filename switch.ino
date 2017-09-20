#include "switch.h"

void setup()
{
  debug_setup();
  switch_setup();
  led_setup();
  reset_setup();
  irrecv_setup();
  mdns_setup();
  wifi_setup();
  server_setup();
}

void loop()
{
  if (irrecv_loop()) {
    return;
  } else if (wifi_loop()) {
    return;
  }
  server_loop();
}

void switch_setup()
{
  for (int i = 0; i < SWITCH_COUNT; i++) {
    pinMode(SWITCHES[i], OUTPUT);
    digitalWrite(SWITCHES[i], SWITCH_OFF);
  }
}

void switch_reset()
{
  debug_println(F("[DEBUG] Switch RESET: "));
  led_reset();
  WiFi.begin("");
  WiFi.disconnect();
  delay(5000);
  ESP.reset();
}

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
