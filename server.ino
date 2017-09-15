#include "switch.h"
#include <ESP8266WebServer.h>

static const char *TYPE_JSON = "application/json";
static const char *TYPE_HTML = "text/html";
static const char *TYPE_CSS = "text/css";
static const char *TYPE_JS = "application/javascript";
static const char *HEADER_CACHE = "Cache-Control";
static const char *CACHE_NONE = "no-cache";
static const char *CACHE_DAY = "max-age=86400";
static const char *ERROR_404 = "{\"success\":0,\"message\":\"Not Found\"}";
static const char *ERROR_422 = "{\"success\":0,\"message\":\"Unprocessable Entity\"}";
static const char *HTML_HEAD = "<!DOCTYPE HTML>\n<html>\n<head>\n<meta charset='utf-8'>\n\
<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n<title>智能开关</title>\n\
<link href='/style.css' rel='stylesheet'>\n</head>\n<body>\n<h1>智能开关</h1>\n";
static const char *HTML_FOOT = "<script src='/turn.js'></script>\n</body>\n</html>";
static const char *STYLE_CSS = "*{margin:0;padding:0}body{background-color:#f8f8f8}h1{text-align:center;margin:1em}\
h2{margin:.77em 0 .3em 0;padding:0 15px;color:#999;font-size:14px}\
dl{margin:0;background-color:#FFF;line-height:1.47;font-size:17px;overflow:hidden;position:relative}\
dl:before{content:'';position:absolute;left:0;top:0;right:0;height:1px;border-top:1px solid #e5e5e5;\
color:#e5e5e5;transform-origin:0 0;transform:scaleY(0.5);z-index:2}\
dl:after{content:'';position:absolute;left:0;bottom:0;right:0;height:1px;border-bottom:1px solid #e5e5e5;\
color:#e5e5e5;transform-origin:0 100%;transform:scaleY(0.5);z-index:2}\
div{padding:10px 15px;position:relative;display:flex;align-items:center}div:first-child:before{display:none}\
div:before{content:'';position:absolute;left:15px;top:0;right:0;height:1px;border-top:1px solid #e5e5e5;\
color:#e5e5e5;transform-origin:0 0;transform:scaleY(0.5);z-index:2}\
dt{flex:1}dd{font-size:0;text-align:right;color:#999}dd form{display:inline-block}\
dd label{position:relative;display:inline-block;width:60px;height:34px}dd label input{display:none}\
dd span{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background-color:#ccc;border-radius:34px}\
dd span:before{position:absolute;content:'';height:26px;width:26px;left:4px;bottom:4px;background-color:white;border-radius:50%}\
dd span.on{background-color:#2196F3;box-shadow:0 0 1px #2196F3}dd span.on:before{transform:translateX(26px)}\
form>input{width:90%;border:0;background-color:#1AAD19;position:relative;display:block;margin:0 auto;padding:0 14px;\
box-sizing:border-box;font-size:18px;text-align:center;text-decoration:none;color:#FFF;line-height:2.56;border-radius:5px;overflow:hidden}\
form>input.off{background-color:#E64340}form>input:after{content:'';width:200%;height:200%;position:absolute;top:0;\
left:0;border:1px solid rgba(0,0,0,0.2);transform:scale(0.5);transform-origin:0 0;box-sizing:border-box;border-radius:10px}\
form>input{margin-top:15px}.hidden{display:none}";
static const char *TURN_JS = "function turn(t){var f=t.parentNode,d=new FormData();\
for(var i=0;i<f.children.length;i++){var v=f.children[i];if(v.name!=undefined&&v.name!='')d.append(v.name,v.value)}\
var r=new XMLHttpRequest();r.onreadystatechange=function(){if(r.readyState==4&&r.status==200){\
var o=JSON.parse(r.responseText);if(o.success!=undefined&&o.success==1){\
var on=false,off=false;for(var i=0;i<o.switches.length;i++){\
var p=document.getElementById('switch-'+o.switches[i].switch),s=o.switches[i].state==1;\
p.querySelector('input[name=state]').value=s?0:1;p.querySelector('span').className=s?'on':'';\
on=on||!s;off=off||s}document.getElementById('switch-on').className=on?'':'hidden';\
document.getElementById('switch-off').className=off?'':'hidden'}}};\
r.open(f.method,f.action);r.send(d);return false}\
function turn2(t){return turn(t.parentNode)}";

static ESP8266WebServer server(80);

void server_setup()
{
  server.on("/", server_root);
  server.on("/style.css", server_style_css);
  server.on("/turn.js", server_turn_js);
  server.on("/switch", HTTP_GET, server_get_switch);
  server.on("/switch", HTTP_POST, server_post_switch);
}

void server_loop()
{
  server.handleClient();
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

static void server_root()
{
  bool can_on, can_off;
  String content = String(HTML_HEAD);

  DPRINTLN("[DEBUG] Server receive get home page");
  server.sendHeader(HEADER_CACHE, CACHE_NONE);
  can_on = can_off = false;
  content += "<h2>开关列表</h2>\n<dl>\n";
  for (int i = 0; i < SWITCH_COUNT; i++) {
    bool state = (digitalRead(SWITCHES[i]) == SWITCH_ON);
    String name = String(i + 1);
    content += "<div>\n<dt>开关 #" + name + "</dt>\n<dd>";
    content += server_form1(name, !state, state ? "关闭" : "打开");
    content += "</dd>\n</div>\n";
    can_on = can_on || !state;
    can_off = can_off || state;
  }

  content += "</dl>\n<h2>批量开关</h2>\n";
  content += server_form2(can_on, true, "全部打开");
  content += server_form2(can_off, false, "全部关闭");
  content += HTML_FOOT;
  server.send(200, TYPE_HTML, content);
}

static void server_style_css()
{
  DPRINTLN("[DEBUG] Server receive get style.css");
  server.sendHeader(HEADER_CACHE, CACHE_DAY);
  server.send(200, TYPE_CSS, STYLE_CSS);
}

static void server_turn_js()
{
  DPRINTLN("[DEBUG] Server receive get turn.js");
  server.sendHeader(HEADER_CACHE, CACHE_DAY);
  server.send(200, TYPE_JS, TURN_JS);
}

static void server_get_switch()
{
  String content = String("{\"success\":1,");

  server.sendHeader(HEADER_CACHE, CACHE_NONE);
  if (server.hasArg("switch")) {
    String name = server.arg("switch");
    int i = name.toInt() - 1;
    if (i < 0 || i >= SWITCH_COUNT) {
      server.send(404, TYPE_JSON, ERROR_404);
      return;
    }
    DPRINTLN("[DEBUG] Server receive get switch #" + name);
    content += "\"switch\":" + name + ",\"state\":";
    content += (digitalRead(SWITCHES[i]) == SWITCH_ON) ? "1" : "0";
    content += "}";
    DPRINTLN("[DEBUG] Server send: " + content);
    server.send(200, TYPE_JSON, content);
    return;
  }
  DPRINTLN("[DEBUG] Server receive get switches");
  content += server_get_switches() + "}";
  DPRINTLN("[DEBUG] Server send: " + content);
  server.send(200, TYPE_JSON, content);
}

static void server_post_switch()
{
  String content = String("{\"success\":1,");

  server.sendHeader(HEADER_CACHE, CACHE_NONE);
  if (server.hasArg("state")) {
    String state = server.arg("state");
    if (state == "1" || state == "0") {
      bool b = (state == "1");
      if (server.hasArg("switch")) {
        String name = server.arg("switch");
        int i = name.toInt() - 1;
        if (i < 0 || i >= SWITCH_COUNT) {
          server.send(404, TYPE_JSON, ERROR_404);
          return;
        }
        content += "\"switch\":" + name + ",\"state\":";
        DPRINTLN("[DEBUG] Server receive post switch #" + name + " to " + state);
        led_switch();
        switch_turn(i, b);
        content += state;
        if (server.hasArg("switches")) {
          content += "," + server_get_switches();
        }
        content += "}";
        DPRINTLN("[DEBUG] Server send: " + content);
        server.send(200, TYPE_JSON, content);
        return;
      }
      content += "\"switches\":[";
      DPRINTLN("[DEBUG] Server receive post switches to " + state);
      led_switch();
      for (int i = 0; i < SWITCH_COUNT; i++) {
        switch_turn(i, b);
        content += "{\"switch\":" + String(i + 1) + ",\"state\":" + state + "}";
        if (i < SWITCH_COUNT - 1) {
          content += ",";
        }
      }
      content += "]}";
      DPRINTLN("[DEBUG] Server send: " + content);
      server.send(200, TYPE_JSON, content);
      return;
    }
  }
  server.send(422, TYPE_JSON, ERROR_422);
}

static void server_turn(int i, String state, String content)
{
  bool b = (state == "1");
  led_switch();
  switch_turn(i, b);
  content += state + "}";
  DPRINTLN("[DEBUG] Server send: " + content);
  server.send(200, TYPE_JSON, content);
}

static String server_get_switches()
{
  String content = String("\"switches\":[");
  for (int i = 0; i < SWITCH_COUNT; i++) {
    content += "{\"switch\":" + String(i + 1) + ",\"state\":";
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
  content += i + "'>\n<input type='hidden' name='switches' value='1'>\n";
  content += "<input type='hidden' name='switch' value='" + i + "'>\n";
  content += "<input type='hidden' name='state' value='";
  content += state ? "1" : "0";
  content += "'>\n";
  content += "<label><input type='submit' value='" + name + "' onclick='return turn2(this)'><span";
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
  content += ">\n<input type='hidden' name='state' value='";
  content += state ? "1" : "0";
  content += "'>\n";
  content += "<input type='submit' value='" + name + "'";
  if (!state) {
    content += " class='off'";
  }
  content += " onclick='return turn(this)'>\n</form>\n";
  return content;
}

