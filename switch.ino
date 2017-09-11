#include <ESP8266WiFi.h> // @see https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <IRrecv.h> // @see https://github.com/markszabo/IRremoteESP8266

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
#define setup_debug() Serial.begin(115200);  \
  Serial.println()
#define DPRINT(...)    Serial.print(__VA_ARGS__)
#define DPRINTLN(...)  Serial.println(__VA_ARGS__)
#else
#define setup_debug()
#define DPRINT(...)
#define DPRINTLN(...)
#endif

static const uint8_t SWITCHS[] = {D1, D2, D6, D7, D0, D3, D9, D10};
static const uint8_t IR_RECV = D5;
static const uint8_t WIFI_LED = D4;
static const uint8_t RESET_BTN = D3;

static const unsigned int SWITCH_IRCODES[] = {
  0xFF30CF, 0xFF18E7, 0xFF7A85, 0xFF10EF,
  0xFF38C7, 0xFF5AA5, 0xFF42BD, 0xFF4AB5
};
static const unsigned int OFF_IRCODE = 0xFF6897;
static const unsigned int ON_IRCODE = 0xFF9867;
static const unsigned int RESET_IRCODE = 0xFF906F;

static IRrecv irrecv(IR_RECV);
static decode_results ir_results;

static const uint8_t WIFI_CONNECTING = 0;
static const uint8_t WIFI_CONNECTED = 1;
static const uint8_t WIFI_DISCONNECTED = 2;
static const uint8_t WIFI_CONFIG = 3;

static uint8_t wifi_state;

static unsigned int led_ticker_counter;
static unsigned int led_ticker_count;
static Ticker led_ticker;

static const char *JSON = "application/json";
static const char *ERROR_404 = "{\"success\":0,\"message\":\"Not Found\"}";
static const char *ERROR_422 = "{\"success\":0,\"message\":\"Unprocessable Entity\"}";
static ESP8266WebServer server(80);

void setup()
{
  setup_debug();
  setup_switchs();
  setup_led();
  setup_reset();
  setup_irrecv();
  setup_wifi();
  setup_server();
}

void loop()
{
  if (handle_irrecv()) {
    return;
  } else if (handle_wifi()) {
    return;
  }
  server.handleClient();
}

void setup_switchs()
{
  for (int i = 0; i < SWITCH_COUNT; i++) {
    pinMode(SWITCHS[i], OUTPUT);
    digitalWrite(SWITCHS[i], SWITCH_OFF);
  }
}

void setup_led()
{
  pinMode(WIFI_LED, OUTPUT);
}

void setup_reset()
{
  pinMode(RESET_BTN, INPUT);
  attachInterrupt(digitalPinToInterrupt(RESET_BTN), handle_reset, FALLING);
}

void setup_irrecv()
{
  irrecv.enableIRIn();
  delay(10);
}

/**
   Wifi 配置
*/
void setup_wifi()
{
  WiFi.mode(WIFI_STA);
  if (WiFi.SSID() == "") {
    wifi_config(); // 没有配置，开始配置
  } else {
    wifi_connect(); // 有配置，连接
  }
}

void setup_server()
{
  server.on("/", handle_server_root);
  server.on("/switch", HTTP_GET, handle_server_get_switch);
  server.on("/switch", HTTP_POST, handle_server_post_switch);
  //server.onNotFound(handle_server_not_found);
}

void handle_reset()
{
  DPRINTLN("[DEBUG] Push reset button");
  switch_reset();
}

bool handle_irrecv()
{
  if (irrecv.decode(&ir_results)) {
    unsigned int ircode = ir_results.value;
    DPRINT("[DEBUG] IR get code: 0x");
    DPRINTLN(ircode, HEX);
    switch (ircode) {
      case OFF_IRCODE:
        DPRINTLN("[DEBUG] IR off");
        for (int i = 0; i < SWITCH_COUNT; i++) {
          switch_turn(i, false);
        }
        break;
      case ON_IRCODE:
        DPRINTLN("[DEBUG] IR on");
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

bool handle_wifi()
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

void handle_server_root()
{
  bool can_on, can_off;

  DPRINTLN("[DEBUG] Server receive get html");

  String response = String("<!DOCTYPE HTML>\r\n<html><head><meta charset='utf-8'>\r\n");
  response += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>\r\n";
  response += "<title>智能开关</title><style>form{display:inline-block}</style></head><body>\r\n";

  can_on = can_off = false;
  for (int i = 0; i < SWITCH_COUNT; i++) {
    bool state = (digitalRead(SWITCHS[i]) == SWITCH_ON);
    String name = String(i + 1);
    response += "<br><br>\r\n开关 #" + name + "：";
    if (state) {
      response += "已打开 " + make_form(name, 0, "关闭");
    } else {
      response += "已关闭 " + make_form(name, 1, "打开");
    }
    can_on = can_on || !state;
    can_off = can_off || state;
  }

  if (can_on) {
    response += "<br><br>\r\n有开关已关闭，可以 " + make_form("", 1, "全部打开");
  }
  if (can_off) {
    response += "<br><br>\r\n有开关已打开，可以 " + make_form("", 0, "全部关闭");
  }

  response += "<iframe id='backend' name='backend' style='display:none'></iframe></body></html>\r\n";
  server.send(200, "text/html", response);
}

void handle_server_get_switch()
{
  String content = String("{\"success\":1,");
  if (server.hasArg("switch")) {
    String name = server.arg("switch");
    int i = name.toInt() - 1;
    if (i < 0 || i >= SWITCH_COUNT) {
      // not found
      server.send(404, JSON, ERROR_404);
      return;
    }
    DPRINTLN("[DEBUG] Server receive get switch #" + name);
    content += "\"switch\":" + name + ",\"state\":";
    content += (digitalRead(SWITCHS[i]) == SWITCH_ON) ? "1" : "0";
    content += "}";
    DPRINTLN("[DEBUG] Server send: " + content);
    server.send(200, JSON, content);
    return;
  }
  DPRINTLN("[DEBUG] Server receive get switchs");
  content += "\"switches\":[";
  for (int i = 0; i < SWITCH_COUNT; i++) {
    content += "{\"switch\":" + String(i + 1) + ",\"state\":";
    content += (digitalRead(SWITCHS[i]) == SWITCH_ON) ? "1" : "0";
    content += "}";
    if (i < SWITCH_COUNT - 1) {
      content += ",";
    }
  }
  content += "]}";
  DPRINTLN("[DEBUG] Server send: " + content);
  server.send(200, JSON, content);
}

void handle_server_post_switch()
{
  String content = String("{\"success\":1,");
  if (server.hasArg("switch")) {
    String name = server.arg("switch");
    int i = name.toInt() - 1;
    if (i < 0 || i >= SWITCH_COUNT) {
      // not found
      server.send(404, JSON, ERROR_404);
      return;
    }
    content += "\"switch\":" + name + ",\"state\":";
    if (server.hasArg("state")) {
      String state = server.arg("state");
      DPRINTLN("[DEBUG] Server receive post switch #" + name + " to " + state);
      switch (get_state_from_str(state)) {
        case 0:
          switch_turn(i, false);
          content += "0}";
          DPRINTLN("[DEBUG] Server send: " + content);
          server.send(200, JSON, content);
          return;
        case 1:
          switch_turn(i, true);
          content += "1}";
          DPRINTLN("[DEBUG] Server send: " + content);
          server.send(200, JSON, content);
          return;
      }
    }
    // error
    server.send(422, JSON, ERROR_422);
    return;
  }
  content += "\"switches\":[";
  if (server.hasArg("state")) {
    String state = server.arg("state");
    DPRINTLN("[DEBUG] Server receive post switchs to " + state);
    switch (get_state_from_str(state)) {
      case 0:
        for (int i = 0; i < SWITCH_COUNT; i++) {
          switch_turn(i, false);
          content += "{\"switch\":" + String(i + 1) + ",\"state\":0}";
          if (i < SWITCH_COUNT - 1) {
            content += ",";
          }
        }
        content += "]}";
        DPRINTLN("[DEBUG] Server send: " + content);
        server.send(200, JSON, content);
        return;
      case 1:
        for (int i = 0; i < SWITCH_COUNT; i++) {
          switch_turn(i, true);
          content += "{\"switch\":" + String(i + 1) + ",\"state\":1}";
          if (i < SWITCH_COUNT - 1) {
            content += ",";
          }
        }
        content += "]}";
        DPRINTLN("[DEBUG] Server send: " + content);
        server.send(200, JSON, content);
        return;
    }
  }
  // error
  server.send(422, JSON, ERROR_422);
}

void switch_reset()
{
  bool ret;
  DPRINT("[DEBUG] Switch RESET: ");
  led_set(true, 100, 0);
  ret = ESP.eraseConfig();
  DPRINTLN(ret);
  WiFi.begin("");
  WiFi.disconnect();
  delay(5000);
  //  ESP.restart();
  ESP.reset();
}

bool switch_toggle(unsigned int i)
{
  if (i < SWITCH_COUNT) {
    int data = !digitalRead(SWITCHS[i]);
    bool state = (data == SWITCH_ON);
    DPRINT("[DEBUG] Switch #");
    DPRINT(i + 1);
    DPRINTLN(state ? " on" : " off");
    digitalWrite(SWITCHS[i], data);
    return state ? true : false;
  }
  return false;
}

bool switch_turn(unsigned int i, bool state)
{
  if (i < SWITCH_COUNT) {
    int data = digitalRead(SWITCHS[i]);
    if (state) {
      if (data == SWITCH_OFF) {
        DPRINT("[DEBUG] Switch #");
        DPRINT(i + 1);
        DPRINTLN(" on");
        digitalWrite(SWITCHS[i], SWITCH_ON);
        return true;
      }
    } else if (data == SWITCH_ON) {
      DPRINT("[DEBUG] Switch #");
      DPRINT(i + 1);
      DPRINTLN(" off");
      digitalWrite(SWITCHS[i], SWITCH_OFF);
      return true;
    }
  }
  return false;
}

void led_tick()
{
  int state = !digitalRead(WIFI_LED);
  if (state == LOW && (led_ticker_count == 0 || ++led_ticker_counter % led_ticker_count == 0)) {
    digitalWrite(WIFI_LED, LOW);
  } else {
    digitalWrite(WIFI_LED, HIGH);
  }
}

void led_set(bool state, int ms, int count)
{
  if (ms == 0) {
    led_ticker.detach();
    digitalWrite(WIFI_LED, state ? LOW : HIGH);
  } else if (state) {
    float s = ms / 1000.0;
    led_ticker_count = count;
    led_ticker.attach(s, led_tick);
  }
}

/**
   连接已配置的 Wifi
*/
void wifi_connect()
{
  DPRINTLN("[DEBUG] Wifi connects: " + WiFi.SSID());
  wifi_state = WIFI_CONNECTING;
  WiFi.begin();
  led_set(true, 200, 0);
}

/**
   配置 Wifi 信息
*/
void wifi_config()
{
  DPRINTLN("[DEBUG] Wifi config");
  wifi_state = WIFI_CONFIG;
  WiFi.beginSmartConfig();
  led_set(true, 500, 0);
}

/**
   配置 Wifi 信息成功
*/
void wifi_received()
{
  DPRINTLN("[DEBUG] Wifi config received");
  wifi_connect(); // 连网
}

/**
   密码错误，Wifi 连接失败
*/
void wifi_failed()
{
  DPRINTLN("[DEBUG] Wifi connect failed");
  wifi_config();  // 配网
}

/**
   Wifi 未连接
*/
void wifi_disconnected()
{
  DPRINTLN("[DEBUG] Wifi disconnected");
  wifi_state = WIFI_DISCONNECTED;
  led_set(true, 200, 10);
}

/**
   Wifi 连接成功
*/
void wifi_connected()
{
  DPRINT("[Debug] Wifi connected, IP address: ");
  DPRINTLN(WiFi.localIP());
  wifi_state = WIFI_CONNECTED;
  led_set(true, 0, 0);
  server_start();
}

void server_start()
{
  server.begin();
  DPRINT("[DEBUG] Server start: http://");
  DPRINT(WiFi.localIP());
  DPRINTLN("/");
}

void server_stop()
{
  server.stop();
  DPRINTLN("[DEBUG] Server stop");
}

int get_state_from_str(String str)
{
  if (str.equalsIgnoreCase("on")) {
    return 1;
  } else if (str.equalsIgnoreCase("off")) {
    return 0;
  }
  return -1;
}

String make_form(String i, int state, String name)
{
  String content = String("<form action='/switch' method='post' target='backend'>");
  if (i != "") {
    content += "<input type='hidden' name='switch' value='" + i + "'>";
  }
  if (state != -1) {
    content += "<input type='hidden' name='state' value='";
    content += state ? "on" : "off";
    content += "'>";
  }
  content += "<input type='submit' value='" + name + "' onclick='this.parentElement.submit();window.location.reload();return false'></form><br>\r\n";
  return content;
}

