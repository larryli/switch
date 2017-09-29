///
// Wifi
//

typedef enum {
  WIFI_CONNECTING,
  WIFI_CONNECTED,
  WIFI_DISCONNECTED,
  WIFI_CONFIG,
} WifiState;

static WifiState _wifi_state; // 是否在配网
static uint8_t _wifi_retry;

#define WIFI_MAXRETRY 3

///
// Wifi 配置
//
void wifi_setup()
{
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.hostname(mdns_name);
  _wifi_retry = 0;
  _wifi_state = WIFI_CONNECTING;
  if (WiFi.SSID() == "") {
    switch_event(EVENT_CONFIG); // 没有配置，开始配置
  } else {
    switch_event(EVENT_CONNECTING); // 有配置，连接
  }
}

bool wifi_loop()
{
  switch (_wifi_state) {
    case WIFI_CONNECTING:
      switch (WiFi.status()) {
        case WL_CONNECTED:
          switch_event(EVENT_CONNECTED);
          return true;
        case WL_CONNECT_FAILED:
          switch_event(EVENT_FAILED);
          return true;
        case WL_NO_SSID_AVAIL:
          switch_event(EVENT_DISCONNECTED);
          return true;
      }
      break;
    case WIFI_CONNECTED:
      if (WiFi.status() == WL_DISCONNECTED) {
        switch_event(EVENT_DISCONNECTED);
        return true;
      }
      break;
    case WIFI_DISCONNECTED:
      if (WiFi.status() == WL_CONNECTED) {
        switch_event(EVENT_CONNECTED);
        return true;
      }
      break;
    case WIFI_CONFIG:
      if (WiFi.smartConfigDone()) {
        switch_event(EVENT_RECEIVED);
        return true;
      }
      break;
  }
  return false;
}

void wifi_event(const Event e)
{
  switch (e) {
    case EVENT_CONNECTING: // 连接已配置的 Wifi
      debug_print(F("[DEBUG] Wifi connect: "));
      debug_println(WiFi.SSID());
      _wifi_state = WIFI_CONNECTING;
      WiFi.begin();
      return;
    case EVENT_CONFIG: // 配置 Wifi 信息
      debug_println(F("[DEBUG] Wifi config"));
      _wifi_state = WIFI_CONFIG;
      _wifi_retry = WIFI_MAXRETRY;
      WiFi.persistent(true);
      WiFi.beginSmartConfig();
      return;
    case EVENT_RECEIVED: // 配置 Wifi 信息成功
      debug_println(F("[DEBUG] Wifi config received"));
      WiFi.persistent(false);
      switch_event(EVENT_CONNECTING); // 连网
      return;
    case EVENT_FAILED: // 密码错误，Wifi 连接失败
      debug_println(F("[DEBUG] Wifi connect failed"));
      if (++_wifi_retry < WIFI_MAXRETRY) {
        switch_event(EVENT_CONNECTING); // 连网
        return;
      }
      switch_event(EVENT_CONFIG); // 配网
      return;
    case EVENT_DISCONNECTED: // Wifi 未连接
      debug_println(F("[DEBUG] Wifi disconnected"));
      _wifi_state = WIFI_DISCONNECTED;
      return;
    case EVENT_CONNECTED: // Wifi 连接成功
      debug_print(F("[DEBUG] Wifi connected, IP address: "));
      debug_println(WiFi.localIP());
      _wifi_state = WIFI_CONNECTED;
      _wifi_retry = 0;
      return;
  }
}
