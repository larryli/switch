///
// HTTP 服务
//

///
// 使用 Flash 保存字符串，节省 RAM 空间
// @see http://espressif.com/sites/default/files/documentation/save_esp8266ex_ram_with_progmem_cn.pdf
//
static const char ERROR_404[] PROGMEM = "{\"success\":0,\"message\":\"Not Found\"}";
static const char ERROR_422[] PROGMEM = "{\"success\":0,\"message\":\"Unprocessable Entity\"}";
static const char HTML_HEAD[] PROGMEM = "<!DOCTYPE HTML>\n<html>\n<head>\n<meta charset='utf-8'>\n\
<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n<title>智能开关</title>\n\
<link href='/style.css' rel='stylesheet'>\n</head>\n<body>\n<h1>智能开关</h1>\n";
static const char HTML_FOOT[] PROGMEM = "<script src='/turn.js'></script>\n</body>\n</html>";

// CSS & JS gzip data
#include "server_data.h"

static AsyncWebServer server(80);
AsyncEventSource events("/events");

///
// HTTP 服务配置
//
void server_setup()
{
  server.on("/", server_root);
  server.on("/style.css", [](AsyncWebServerRequest *request) {
    debug_println(F("[DEBUG] Server receive get style.css"));
    server_static(request, "text/css", STYLE_CSS_GZ_DATA, STYLE_CSS_GZ_LEN);
  });
  server.on("/turn.js", [](AsyncWebServerRequest *request) {
    debug_println(F("[DEBUG] Server receive get turn.js"));
    server_static(request, "application/javascript", TURN_JS_GZ_DATA, TURN_JS_GZ_LEN);
  });
  server.on("/switch", HTTP_GET, server_get_switch);
  server.on("/switch", HTTP_POST, server_post_switch);
  events.onConnect([](AsyncEventSourceClient *client){
    String content = String("{\"success\":1");
    content += server_get_switches();
    content += "}";
    client->send(content.c_str(), NULL, millis());
  });
  server.addHandler(&events);
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });
  server.begin();
  debug_println(F("[DEBUG] Server start"));
}

void server_update()
{
  String content = String("{\"success\":1");
  content += server_get_switches();
  content += "}";
  events.send(content.c_str(), NULL, millis());
}

static void server_root(AsyncWebServerRequest *request)
{
  bool can_on, can_off;
  String content = String(FPSTR(HTML_HEAD));

  debug_println(F("[DEBUG] Server receive get home page"));
  can_on = can_off = false;
  content += "<h2>开关列表</h2>\n<dl>\n";
  for (int i = 0; i < SWITCH_COUNT; i++) {
    bool state = (digitalRead(SWITCHES[i]) == SWITCH_ON);
    String name = String(i + 1);
    content += "<div>\n<dt>开关 #";
    content += name;
    content += "</dt>\n<dd>";
    content += server_form1(name, !state, state ? "关闭" : "打开");
    content += "</dd>\n</div>\n";
    can_on = can_on || !state;
    can_off = can_off || state;
  }

  content += "</dl>\n<h2>批量开关</h2>\n";
  content += server_form2(can_on, true, "全部打开");
  content += server_form2(can_off, false, "全部关闭");
  content += FPSTR(HTML_FOOT);
  request->send(200, "text/html", content);
}

static void server_get_switch(AsyncWebServerRequest *request)
{
  String content = String("{\"success\":1");

  if (request->hasArg("switch")) {
    String name = request->arg("switch");
    int i = name.toInt() - 1;
    if (i < 0 || i >= SWITCH_COUNT) {
      request->send(404, "application/json", FPSTR(ERROR_404));
      return;
    }
    debug_print(F("[DEBUG] Server receive get switch #"));
    debug_println(name);
    content += ",\"switch\":";
    content += name;
    content += ",\"state\":";
    content += (digitalRead(SWITCHES[i]) == SWITCH_ON) ? "1" : "0";
    content += "}";
    debug_print(F("[DEBUG] Server send: "));
    debug_println(content);
    request->send(200, "application/json", content);
    return;
  }
  debug_println(F("[DEBUG] Server receive get switches"));
  content += server_get_switches();
  content += "}";
  debug_print(F("[DEBUG] Server send: "));
  debug_println(content);
  request->send(200, "application/json", content);
}

static void server_post_switch(AsyncWebServerRequest *request)
{
  String content = String("{\"success\":1");
  int ret_type = 1;

  if (request->hasArg("switches")) {
    String s = request->arg("switches");
    ret_type = (s == "0") ? 0 : 2;
  }
  if (request->hasArg("state")) {
    String state = request->arg("state");
    bool b = (state == "0") ? false : true;

    if (request->hasArg("switch")) {
      String name = request->arg("switch");
      int i = name.toInt() - 1;

      if (i < 0 || i >= SWITCH_COUNT) {
        request->send(404, "application/json", FPSTR(ERROR_404));
        return;
      }
      debug_print(F("[DEBUG] Server receive post switch #"));
      debug_print(name);
      debug_print(F(" to "));
      debug_println(state);
      led_switch();
      switch_turn(i, b);
      oled_refresh();
      if (ret_type > 0) {
        content += ",\"switch\":";
        content += name;
        content += ",\"state\":";
        content += state;
        if (ret_type > 1) {
          content += server_get_switches();
        }
      }
      content += "}";
      debug_print(F("[DEBUG] Server send: "));
      debug_println(content);
      request->send(200, "application/json", content);
      server_update();
      return;
    }

    debug_print(F("[DEBUG] Server receive post switches to "));
    debug_println(state);
    led_switch();
    for (int i = 0; i < SWITCH_COUNT; i++) {
      switch_turn(i, b);
    }
    oled_refresh();
    if (ret_type > 0) {
      content += server_get_switches();
    }
    content += "}";
    debug_print(F("[DEBUG] Server send: "));
    debug_println(content);
    request->send(200, "application/json", content);
    server_update();
    return;
  }
  request->send(422, "application/json", FPSTR(ERROR_422));
}

static String server_get_switches()
{
  String content = String(",\"switches\":[");
  for (int i = 0; i < SWITCH_COUNT; i++) {
    content += "{\"switch\":";
    content += String(i + 1);
    content += ",\"state\":";
    content += (digitalRead(SWITCHES[i]) == SWITCH_ON) ? "1" : "0";
    content += "}";
    if (i < SWITCH_COUNT - 1) {
      content += ",";
    }
  }
  content += "]";
  return content;
}

static String server_form1(String i, bool state, String name)
{
  String content = String("<form action='/switch' method='post' id='switch-");
  content += i;
  content += "'>\n<input type='hidden' name='switches' value='1'>\n<input type='hidden' name='switch' value='";
  content += i;
  content += "'>\n<input type='hidden' name='state' value='";
  content += state ? "1" : "0";
  content += "'>\n";
  content += "<label><input type='submit' value='";
  content += name;
  content += "' onclick='return turn(this,1)'><span";
  if (!state) {
    content += " class='on'";
  }
  content += "></span></label>\n</form>\n";
  return content;
}

static String server_form2(bool show, bool state, String name)
{
  String content = String("<form action='/switch' method='post' id='switch-");
  content += state ? "on" : "off";
  content += "'";
  if (!show) {
    content += " class='hidden'";
  }
  content += ">\n<input type='hidden' name='switches' value='1'><input type='hidden' name='state' value='";
  content += state ? "1" : "0";
  content += "'>\n<input type='submit' value='";
  content += name;
  content += "'";
  if (!state) {
    content += " class='off'";
  }
  content += " onclick='return turn(this)'>\n</form>\n";
  return content;
}

static void server_static(AsyncWebServerRequest *request, const String& type, const uint8_t * data, size_t len)
{
  String etag = String(len);

  if (request->header("If-Modified-Since") == LAST_MODIFIED) {
    debug_println(F("[DEBUG] Not modified"));
    request->send(304); // Not modified
  } else if (request->hasHeader("If-None-Match") && request->header("If-None-Match").equals(etag)) {
    AsyncWebServerResponse * response = new AsyncBasicResponse(304); // Not modified

    debug_println(F("[DEBUG] ETAG Not modified"));
    response->addHeader("Cache-Control", "max-age=3600");
    response->addHeader("ETag", etag);
    request->send(response);
  } else {
    AsyncWebServerResponse *response = request->beginResponse_P(200, type, data, len);

    debug_println(F("[DEBUG] GZIP output"));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Last-Modified", LAST_MODIFIED);
    response->addHeader("Cache-Control", "max-age=3600");
    response->addHeader("ETag", etag);
    request->send(response);
  }
}
