///
// Wifi
//

static uint8_t wifi_state;
static uint8_t wifi_retry;

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
  wifi_retry = 0;
  if (WiFi.SSID() == "") {
    wifi_config(); // 没有配置，开始配置
  } else {
    wifi_connect(); // 有配置，连接
  }
}

bool wifi_loop()
{
  switch (wifi_state) {
    case WIFI_CONNECTING:
      switch (WiFi.status()) {
        case WL_CONNECTED:
          wifi_connected();
          return true;
        case WL_CONNECT_FAILED:
          wifi_failed();
          return true;
        case WL_NO_SSID_AVAIL:
          wifi_disconnected();
          return true;
      }
      break;
    case WIFI_CONNECTED:
      if (WiFi.status() == WL_DISCONNECTED) {
        wifi_disconnected();
        return true;
      }
      break;
    case WIFI_DISCONNECTED:
      if (WiFi.status() == WL_CONNECTED) {
        wifi_connected();
        return true;
      }
      break;
    case WIFI_CONFIG:
      if (WiFi.smartConfigDone()) {
        wifi_received();
        return true;
      }
      break;
  }
  return false;
}

bool wifi_is_connected()
{
  return wifi_state == WIFI_CONNECTED;
}

uint8_t wifi_get_state()
{
  return wifi_state;
}

///
// 连接已配置的 Wifi
//
static void wifi_connect()
{
  debug_print(F("[DEBUG] Wifi connect: "));
  debug_println(WiFi.SSID());
  wifi_state = WIFI_CONNECTING;
  WiFi.begin();
  led_connect();
  oled_refresh();
}

///
// 配置 Wifi 信息
//
static void wifi_config()
{
  debug_println(F("[DEBUG] Wifi config"));
  wifi_state = WIFI_CONFIG;
  wifi_retry = WIFI_MAXRETRY;
  WiFi.persistent(true);
  WiFi.beginSmartConfig();
  led_config();
  oled_refresh();
}

///
// 配置 Wifi 信息成功
//
static void wifi_received()
{
  debug_println(F("[DEBUG] Wifi config received"));
  WiFi.persistent(false);
  wifi_connect(); // 连网
  oled_refresh();
}

///
// 密码错误，Wifi 连接失败
//
static void wifi_failed()
{
  debug_println(F("[DEBUG] Wifi connect failed"));
  if (++wifi_retry < WIFI_MAXRETRY) {
    wifi_connect();
    return;
  }
  wifi_config();  // 配网
  oled_refresh();
}

///
// Wifi 未连接
//
static void wifi_disconnected()
{
  debug_println(F("[DEBUG] Wifi disconnected"));
  wifi_state = WIFI_DISCONNECTED;
  led_disconnected();
  oled_refresh();
}

///
// Wifi 连接成功
//
static void wifi_connected()
{
  debug_print(F("[DEBUG] Wifi connected, IP address: "));
  debug_println(WiFi.localIP());
  wifi_state = WIFI_CONNECTED;
  wifi_retry = 0;
  led_connected();
  mdns_start();
#ifdef SWITCH_OLED
  oled_qrcode();
  oled_refresh();
#endif
}
