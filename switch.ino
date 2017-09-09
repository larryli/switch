#include <ESP8266WiFi.h>

#define SWITCH_COUNT 2  // 继电器数量
#define SWITCH_TRIG_LOW // 继电器低电平触发
#define SWITCH_DEBUG // 开启 TX0 调试输出

#ifdef SWITCH_TRIG_LOW
  #define SWITCH_ON LOW
  #define SWITCH_OFF HIGH
#else
  #define SWITCH_ON HIGH
  #define SWITCH_OFF LOW
#endif

#ifdef SWITCH_DEBUG
void debug_setup()
{
  Serial.begin(115200);
  Serial.println();
}
#else
void debug_setup()
{
}
#endif

static const uint8_t switchs[] = {D1, D2, D6, D7, D0, D3, D9, D10};
static const uint8_t wifi_led = D4;
static const uint8_t setup_btn = D3; 

void setup() {
  debug_setup();
  setup_switchs();
  pinMode(wifi_led, OUTPUT);
  pinMode(setup_btn, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
}

void setup_switchs()
{
  for (int i = 0; i < SWITCH_COUNT; i++) {
    pinMode(switchs[i], OUTPUT);
    digitalWrite(switchs[i], SWITCH_OFF);
  }
}

