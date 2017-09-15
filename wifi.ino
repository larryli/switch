#include "switch.h"

static const uint8_t WIFI_CONNECTING = 0;
static const uint8_t WIFI_CONNECTED = 1;
static const uint8_t WIFI_DISCONNECTED = 2;
static const uint8_t WIFI_CONFIG = 3;

static uint8_t wifi_state;

///
// Wifi 配置
//
void wifi_setup()
{
  WiFi.mode(WIFI_STA);
  WiFi.hostname(mdns_name);
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

///
// 连接已配置的 Wifi
//
static void wifi_connect()
{
  DPRINTLN("[DEBUG] Wifi connects: " + WiFi.SSID());
  wifi_state = WIFI_CONNECTING;
  WiFi.begin();
  led_connect();
}

///
// 配置 Wifi 信息
//
static void wifi_config()
{
  DPRINTLN("[DEBUG] Wifi config");
  wifi_state = WIFI_CONFIG;
  WiFi.beginSmartConfig();
  led_config();
}

///
// 配置 Wifi 信息成功
//
static void wifi_received()
{
  DPRINTLN("[DEBUG] Wifi config received");
  wifi_connect(); // 连网
}

///
// 密码错误，Wifi 连接失败
//
static void wifi_failed()
{
  DPRINTLN("[DEBUG] Wifi connect failed");
  wifi_config();  // 配网
}

///
// Wifi 未连接
//
static void wifi_disconnected()
{
  DPRINTLN("[DEBUG] Wifi disconnected");
  wifi_state = WIFI_DISCONNECTED;
  led_disconnected();
}

///
// Wifi 连接成功
//
static void wifi_connected()
{
  DPRINT("[DEBUG] Wifi connected, IP address: ");
  DPRINTLN(WiFi.localIP());
  wifi_state = WIFI_CONNECTED;
  led_connected();
  server_start();
  mdns_start();
}

