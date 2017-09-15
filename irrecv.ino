#include "switch.h"
#include <IRrecv.h> // @see https://github.com/markszabo/IRremoteESP8266

static const unsigned int SWITCH_IRCODES[] = {
  0xFF30CF, 0xFF18E7, 0xFF7A85, 0xFF10EF,
  0xFF38C7, 0xFF5AA5, 0xFF42BD, 0xFF4AB5
};
static const unsigned int OFF_IRCODE = 0xFF6897;
static const unsigned int ON_IRCODE = 0xFF9867;
static const unsigned int RESET_IRCODE = 0xFF906F;

static IRrecv irrecv(IR_RECV);
static decode_results ir_results;

void irrecv_setup()
{
  irrecv.enableIRIn();
  delay(10);
}

bool irrecv_loop()
{
  if (irrecv.decode(&ir_results)) {
    unsigned int ircode = ir_results.value;
    DPRINT("[DEBUG] IR get code: 0x");
#ifdef SWITCH_DEBUG
    Serial.println(ircode, HEX);
#endif
    switch (ircode) {
      case OFF_IRCODE:
        DPRINTLN("[DEBUG] IR off");
        led_switch();
        for (int i = 0; i < SWITCH_COUNT; i++) {
          switch_turn(i, false);
        }
        break;
      case ON_IRCODE:
        DPRINTLN("[DEBUG] IR on");
        led_switch();
        for (int i = 0; i < SWITCH_COUNT; i++) {
          switch_turn(i, true);
        }
        break;
      case RESET_IRCODE:
        DPRINTLN("[DEBUG] IR reset");
        switch_reset();
        break;
      default:
        for (int i = 0; i < SWITCH_COUNT; i++) {
          if (ircode == SWITCH_IRCODES[i]) {
            DPRINT("[DEBUG] IR switch #");
            DPRINTLN(i + 1);
            led_switch();
            switch_toggle(i);
          }
        }
        break;
    }
    irrecv.resume();
    delay(100);
    return true;
  }
  return false;
}

