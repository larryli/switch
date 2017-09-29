///
// 重置配网
//

///
// 重置按键配置
//
void reset_setup()
{
  pinMode(RESET_BTN, INPUT);
  attachInterrupt(digitalPinToInterrupt(RESET_BTN), reset_handle, FALLING);
}

void reset_event(const Event e)
{
  if (e != EVENT_RESET) {
    return;
  }
  reset_handle();
}

///
// 系统重置，清除 Wifi 配置，重启
//
static void reset_handle()
{
  if (millis() < 3000) {
    return; // 修正开发板上电时的低电平
  }
  debug_println(F("[DEBUG] RESET: "));
  WiFi.begin("");
  WiFi.disconnect();
  ESP.eraseConfig();
  delay(1000);
  ESP.restart();
}
