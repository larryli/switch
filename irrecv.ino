///
// 红外
//

#ifdef SWITCH_IR 
#include <IRrecv.h> // @see https://github.com/markszabo/IRremoteESP8266

///
// 红外 1~8 按键码
//
static const unsigned int _ircode_switches[] = {
  0xFF30CF, 0xFF18E7, 0xFF7A85, 0xFF10EF,
  0xFF38C7, 0xFF5AA5, 0xFF42BD, 0xFF4AB5
};
// 红外 ⏮ 按键码
#define IRCODE_UP 0xFF22DD
// 红外 ⏭ 按键码
#define IRCODE_DOWN 0xFF02FD
// 红外 ⏯ 按键码
#define IRCODE_SELECT 0xFFC23D
// 红外 0 按键码
#define IRCODE_OFF 0xFF6897
// 红外 100+ 按键码
#define IRCODE_ON 0xFF9867
// 红外 EQ 按键码
#define IRCODE_RESET 0xFF906F

static IRrecv _irrecv(IR_RECV);
static decode_results _ir_results;

///
// 红外配置
//
void irrecv_setup()
{
  _irrecv.enableIRIn();
  delay(10);
}

///
// 红外处理
//
bool irrecv_loop()
{
  if (_irrecv.decode(&_ir_results)) {
    unsigned int ircode = _ir_results.value;

    debug_print(F("[DEBUG] IR get code: 0x"));
    debug_println(ircode, HEX);
    switch (ircode) {
#ifdef SWITCH_OLED
      case IRCODE_UP:
        debug_println(F("[DEBUG] IR up"));
        switch_event(EVENT_UP);
        break;
      case IRCODE_DOWN:
        debug_println(F("[DEBUG] IR down"));
        switch_event(EVENT_DOWN);
        break;
      case IRCODE_SELECT:
        debug_println(F("[DEBUG] IR select"));
        switch_event(EVENT_SELECT);
        break;
#endif
      case IRCODE_OFF:
        debug_println(F("[DEBUG] IR off"));
        switch_event(EVENT_OFF);
        break;
      case IRCODE_ON:
        debug_println(F("[DEBUG] IR on"));
        switch_event(EVENT_ON);
        break;
      case IRCODE_RESET:
        debug_println(F("[DEBUG] IR reset"));
        switch_event(EVENT_RESET);
        break;
      default:
        for (int i = 0; i < SWITCH_COUNT; i++) {
          if (ircode == _ircode_switches[i]) {
            debug_print(F("[DEBUG] IR switch #"));
            debug_println(i + 1);
            switch_event(Event(i + EVENT_1));
            break;
          }
        }
        break;
    }
    _irrecv.resume();
    delay(100);
    return true;
  }
  return false;
}
#endif
