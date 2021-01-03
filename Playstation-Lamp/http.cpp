
/*
  Playstation-Lamp

  (c) 2020 Christian.Lorenz@gromeck.de

  module to handle the HTTP stuff


  This file is part of Playstation-Lamp.

  Playstation-Lamp is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Playstation-Lamp is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Playstation-Lamp.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <ESP8266WebServer.h>
#include "config.h"
#include "http.h"
#include "wifi.h"
#include "mqtt.h"
#include "ntp.h"
#include "led.h"
#include "aoxa.h"

/*
   the web server object
*/
static ESP8266WebServer _WebServer(80);

/*
  time of the last HTTP request
*/
static unsigned long _last_request = 0;

/*
   setup the webserver
*/
void HttpSetup(void)
{
  static String _html_header =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1,user-scalable=no'>"
    "<title>" __TITLE__ "</title>"
    "<link href='/styles.css' rel='stylesheet' type='text/css'>"
    "</head>"
    "<body>"
    "<div class=content>"
    "<div class=header>"
    "<h3>" __TITLE__ "</h3>"
    "<h2>" + String(_config.device.name) + "</h2>"
    "</div>"
    ;
  static String _html_footer =
    "<div class=footer>"
    "<hr>"
    "<a href='https://github.com/gromeck/Playstation-Lamp' target='_blank' style='color:#aaa;'>" __TITLE__ " Version: " GIT_VERSION "</a>"
    "</div>"
    "</div>"
    "</body>"
    "</html>"
    ;

  LogMsg("HTTP: setting up HTTP server");

  /*
     get the current config as a duplicate
  */
  ConfigGet(0, sizeof(CONFIG), &_config);

  _WebServer.onNotFound( []() {
    _last_request = millis();
    if (!StateCheck(STATE_CONFIGURING) && _config.device.password[0] && !_WebServer.authenticate(HTTP_WEB_USER, _config.device.password))
      return _WebServer.requestAuthentication();
      
    if (_WebServer.hasArg("switch"))
      AoxaNextMode();

    _WebServer.send(200, "text/html",
                    _html_header +
                    "<p><form action='/' method='get'><button name='switch' type='submit' class='button switch'>" + AoxaLookupMode(AoxaGetMode()) + "</button></form><p>"
                    "<form action='/config' method='get'><button>Configuration</button></form><p>"
                    "<form action='/info' method='get'><button>Information</button></form><p>"
                    "<form action='/restart' method='get' onsubmit=\"return confirm('Are you sure to restart the device?');\"><button class='button redbg'>Restart</button></form><p>"
                    + _html_footer);
  });

  _WebServer.on("/styles.css", []() {
    _last_request = millis();
    _WebServer.send(200, "text/css",
                    "html, body { background:#ffffff; }"
                    "body { margin:1rem; padding:0; font-familiy:'sans-serif'; color:#202020; text-align:center; font-size:1rem; }"
                    "input { width:90%; font-size:1rem; }"
                    "button { border:0; border-radius:0.3rem; background:#1881ba; color:#ffffff; line-height:2.4rem; font-size:1.2rem; width:100%; -webkit-transition-duration:0.5s; transition-duration:0.5s; cursor:pointer; opacity:0.8; }"
                    "button:hover { opacity:1.0; }"
                    ".switch { padding:2rem; background:#f0f0f0; color:#202020; text-align:center; font-weight: bold; font-size: 3rem; }"
                    ".header { text-align:center; }"
                    ".content { text-align:left; display:inline-block; color:#000000; min-width:340px; }"
                    ".msg { text-align:center; color:#be3731; font-weight:bold; padding:5rem 0; }"
                    ".footer { text-align:right; }"
                    ".greenbg { background:#348f4b; }"
                    ".redbg { background:#a12828; }"
                   );
  });

  _WebServer.on("/config", []() {
    _last_request = millis();
    if (!StateCheck(STATE_CONFIGURING) && _config.device.password[0] && !_WebServer.authenticate(HTTP_WEB_USER, _config.device.password))
      return _WebServer.requestAuthentication();

    /*
       handle configuration changes
    */
#if DBG
    for (int n = 0; n < _WebServer.args(); n++ )
      LogMsg("HTTP: args: %s=%s", _WebServer.argName(n).c_str(), _WebServer.arg(n).c_str());
#endif

    if (_WebServer.hasArg("save")) {
      /*
         take over the configuration parameters
      */
#define CHECK_AND_SET_STRING(type,name) { if (_WebServer.hasArg(#type "_" #name)) strncpy(_config.type.name,_WebServer.arg(#type "_" #name).c_str(),sizeof(_config.type.name) - 1); }
#define CHECK_AND_SET_NUMBER(type,name,minimum,maximum) { if (_WebServer.hasArg(#type "_" #name)) _config.type.name = min(max(atoi(_WebServer.arg(#type "_" #name).c_str()),(int) (minimum)),(int) (maximum)); }
      CHECK_AND_SET_STRING(device, name);
      CHECK_AND_SET_STRING(device, password);
      CHECK_AND_SET_STRING(wifi, ssid);
      CHECK_AND_SET_STRING(wifi, psk);
      CHECK_AND_SET_STRING(ntp, server);
      CHECK_AND_SET_STRING(mqtt, server);
      CHECK_AND_SET_NUMBER(mqtt, port, MQTT_PORT_MIN, MQTT_PORT_MAX);
      CHECK_AND_SET_STRING(mqtt, user);
      CHECK_AND_SET_STRING(mqtt, password);
      CHECK_AND_SET_STRING(mqtt, clientID);
      CHECK_AND_SET_STRING(mqtt, topicPrefix);
      CHECK_AND_SET_NUMBER(aoxa, default_mode, AOXA_MODE_OFF, AOXA_MODE_LAST - 1);
      CHECK_AND_SET_NUMBER(aoxa, fade_speed, AOXA_FADE_SPEED_MIN, AOXA_FADE_SPEED_MAX);
      CHECK_AND_SET_NUMBER(aoxa, flash_speed, AOXA_FLASH_SPEED_MIN, AOXA_FLASH_SPEED_MAX);
      CHECK_AND_SET_NUMBER(aoxa, blink_speed, AOXA_BLINK_SPEED_MIN, AOXA_BLINK_SPEED_MAX);
      CHECK_AND_SET_NUMBER(aoxa, fire_speed, AOXA_FIRE_SPEED_MIN, AOXA_FIRE_SPEED_MAX);

      /*
         write the config back
      */
      ConfigSet(0, sizeof(CONFIG), &_config);
    }

    _WebServer.send(200, "text/html",
                    _html_header +
                    "<form action='/config/device' method='get'><button>Configure Device</button></form><p>"
                    "<form action='/config/wifi' method='get'><button>Configure WiFi</button></form><p>"
                    "<form action='/config/ntp' method='get'><button>Configure NTP</button></form><p>"
                    "<form action='/config/mqtt' method='get'><button>Configure MQTT</button></form><p>"
                    "<form action='/config/leds' method='get'><button>Configure LEDs</button></form><p>"
                    "<form action='/config/reset' method='get' onsubmit=\"return confirm('Are you sure to reset the configuration?');\"><button class='button redbg'>Reset configuration</button></form><p>"
                    "<p><form action='/' method='get'><button>Main Menu</button></form><p>"
                    + _html_footer);
  });

  _WebServer.on("/config/device", []() {
    _last_request = millis();
    _WebServer.send(200, "text/html",
                    _html_header +
                    "<form method='get' action='/config'>"

                    "<fieldset>"
                    "<legend>"
                    "<b>&nbsp;Device&nbsp;</b>"
                    "</legend>"

                    "<b>Name</b>"
                    "<br>"
                    "<input name='device_name' type='text' placeholder='Device name' value='" + String(_config.device.name) + "'>"
                    "<p>"

                    "<b>Web Password</b>"
                    "<br>"
                    "<input name='device_password' type='password' placeholder='Device Password' value='" + String(_config.device.password) + "'>"
                    "<p>"

                    "<b>Note:</b> username for authentication is <b>" HTTP_WEB_USER "</b>"
                    "<p>"
                    "<button name='save' type='submit' class='button greenbg'>Speichern</button>"
                    "</fieldset>"
                    "</form>"
                    "<p><form action='/config' method='get'><button>Configuration Menu</button></form><p>"
                    + _html_footer);
  });

  _WebServer.on("/config/wifi", []() {
    _last_request = millis();
    _WebServer.send(200, "text/html",
                    _html_header +
                    "<form method='get' action='/config'>"
                    "<fieldset>"
                    "<legend>"
                    "<b>&nbsp;WiFi&nbsp;</b>"
                    "</legend>"

                    "<b>SSID</b>"
                    "<br>"
                    "<input name='wifi_ssid' type='text' placeholder='WiFi SSID' value='" + String(_config.wifi.ssid) + "'>"
                    "<p>"

                    "<b>Password</b>"
                    "<br>"
                    "<input name='wifi_psk' type='password' placeholder='WiFi Password' value='" + String(_config.wifi.psk) + "'>"
                    "<p>"

                    "<button name='save' type='submit' class='button greenbg'>Speichern</button>"
                    "</fieldset>"
                    "</form>"
                    "<p><form action='/config' method='get'><button>Configuration Menu</button></form><p>"
                    + _html_footer);
  });

  _WebServer.on("/config/ntp", []() {
    _last_request = millis();
    _WebServer.send(200, "text/html",
                    _html_header +
                    "<form method='get' action='/config'>"
                    "<fieldset>"
                    "<legend>"
                    "<b>&nbsp;NTP&nbsp;</b>"
                    "</legend>"

                    "<b>Server</b>"
                    "<br>"
                    "<input name='ntp_server' type='text' placeholder='NTP server' value='" + String(_config.ntp.server) + "'>"
                    "<p>"

                    "<button name='save' type='submit' class='button greenbg'>Speichern</button>"
                    "</fieldset>"
                    "</form>"
                    "<p><form action='/config' method='get'><button>Configuration Menu</button></form><p>"
                    + _html_footer);
  });

  _WebServer.on("/config/mqtt", []() {
    _last_request = millis();
    _WebServer.send(200, "text/html",
                    _html_header +
                    "<form method='get' action='/config'>"
                    "<fieldset>"
                    "<legend>"
                    "<b>&nbsp;MQTT&nbsp;</b>"
                    "</legend>"

                    "<b>Server</b>"
                    "<br>"
                    "<input name='mqtt_server' type='text' placeholder='MQTT server' value='" + String(_config.mqtt.server) + "'>"
                    "<p>"

                    "<b>Port</b>"
                    "<br>"
                    "<input name='mqtt_port' type='text' placeholder='MQTT port' value='" + String(_config.mqtt.port) + "'>"
                    "<p>"

                    "<b>User (optional)</b>"
                    "<br>"
                    "<input name='mqtt_user' type='text' placeholder='MQTT user' value='" + String(_config.mqtt.user) + "'>"
                    "<p>"

                    "<b>Password (optional)</b>"
                    "<br>"
                    "<input name='mqtt_password' type='text' placeholder='MQTT password' value='" + String(_config.mqtt.password) + "'>"
                    "<p>"

                    "<b>ClientID</b>"
                    "<br>"
                    "<input name='mqtt_clientID' type='text' placeholder='MQTT ClientID' value='" + String(_config.mqtt.clientID) + "'>"
                    "<p>"

                    "<b>TopicPrefix</b>"
                    "<br>"
                    "<input name='mqtt_topicPrefix' type='text' placeholder='MQTT Topic Prefix' value='" + String(_config.mqtt.topicPrefix) + "'>"
                    "<p>"

                    "<button name='save' type='submit' class='button greenbg'>Speichern</button>"
                    "</fieldset>"
                    "</form>"
                    "<p><form action='/config' method='get'><button>Configuration Menu</button></form><p>"
                    + _html_footer);
  });

  _WebServer.on("/config/leds", []() {
    _last_request = millis();

    _WebServer.send(200, "text/html",
                    _html_header +
                    "<form method='get' action='/config'>"

                    "<fieldset>"
                    "<legend>"
                    "<b>&nbsp;LEDs&nbsp;</b>"
                    "</legend>"

                    "<b>Startup Mode</b> "
                    "<br>"
                    "<input name='aoxa_default_mode' type='number' placeholder='Startup Mode' min=" + String(AOXA_MODE_OFF) + " max=" + String(AOXA_MODE_LAST - 1) + " value='" + String(_config.aoxa.default_mode) + "'>"
                    "<p>"

                    "<b>Fade Speed [ms]</b> "
                    "<br>"
                    "<input name='aoxa_fade_speed' type='number' placeholder='LED Fade Speed' min=" + String(AOXA_FADE_SPEED_MIN) + " max=" + String(AOXA_FADE_SPEED_MAX) + " value='" + String(_config.aoxa.fade_speed) + "'>"
                    "<p>"

                    "<b>Flash Speed [ms]</b> "
                    "<br>"
                    "<input name='aoxa_flash_speed' type='number' placeholder='LED Flash Speed' min=" + String(AOXA_FLASH_SPEED_MIN) + " max=" + String(AOXA_FLASH_SPEED_MAX) + " value='" + String(_config.aoxa.flash_speed) + "'>"
                    "<p>"

                    "<b>Blink Speed [ms]</b> "
                    "<br>"
                    "<input name='aoxa_blink_speed' type='number' placeholder='LED Blink Speed' min=" + String(AOXA_BLINK_SPEED_MIN) + " max=" + String(AOXA_BLINK_SPEED_MAX) + " value='" + String(_config.aoxa.blink_speed) + "'>"
                    "<p>"

                    "<b>Fire Speed [ms]</b> "
                    "<br>"
                    "<input name='aoxa_fire_speed' type='number' placeholder='LED Fire Speed' min=" + String(AOXA_FIRE_SPEED_MIN) + " max=" + String(AOXA_FIRE_SPEED_MAX) + " value='" + String(_config.aoxa.fire_speed) + "'>"
                    "<p>"

                    "<button name='save' type='submit' class='button greenbg'>Speichern</button>"
                    "</fieldset>"
                    "</form>"
                    "<p><form action='/config' method='get'><button>Configuration Menu</button></form><p>"
                    + _html_footer);
  });


  _WebServer.on("/config/reset", []() {
    _last_request = millis();
    if (!StateCheck(STATE_CONFIGURING) && _config.device.password[0] && !_WebServer.authenticate(HTTP_WEB_USER, _config.device.password))
      return _WebServer.requestAuthentication();

    /*
       reset the config
    */
    memset(&_config, 0, sizeof(_config));
    ConfigSet(0, sizeof(CONFIG), &_config);

    _WebServer.send(200, "text/html",
                    _html_header +
                    "<div class='msg'>"
                    "Configuration was reset."
                    "<p>"
                    "Wait for the device to come up with an WiFi-AccessPoint, connect to it to configure the device."
                    "</div>"
                    + _html_footer);

    /*
        trigger reboot
    */
    StateChange(STATE_WAIT_BEFORE_REBOOTING);
  });

  _WebServer.on("/info", []() {
    _last_request = millis();
    if (_config.device.password[0] && !_WebServer.authenticate(HTTP_WEB_USER, _config.device.password))
      return _WebServer.requestAuthentication();

    _WebServer.send(200, "text/html",
                    _html_header +
                    "<div class='info'>"
                    "<table style='width:100%'"

                    "<tr>"
                    "<th>" __TITLE__ " Version</th>"
                    "<td>" GIT_VERSION "</td>"
                    "</tr>"

                    "<tr>"
                    "<th>Build Date</th>"
                    "<td>" __DATE__ " " __TIME__ "</td>"
                    "</tr>"

                    "<tr>"
                    "<th>Device Name</th>"
                    "<td>" + String(_config.device.name) + "</td>"
                    "</tr>"

                    "<tr>"
                    "<th>Up since</th>"
                    "<td>" + String(TimeToString(NtpUpSince())) + "</td>"
                    "</tr>"

                    "<tr><th></th><td>&nbsp;</td></tr>"

                    "<tr>"
                    "<th>WiFi SSID</th>"
                    "<td>" + WifiGetSSID() + "</td>"
                    "</tr>"

                    "<tr>"
                    "<th>WiFi RSSI</th>"
                    "<td>" + String(WIFI_RSSI_TO_QUALITY(WifiGetRSSI())) + "% (" + WifiGetRSSI() + "dBm)</td>"
                    "</tr>"

                    "<tr>"
                    "<th>WiFi MAC</th>"
                    "<td>" + WifiGetMacAddr() + "</td>"
                    "</tr>"

                    "<tr>"
                    "<th>WiFi IP Address</th>"
                    "<td>" + WifiGetIpAddr() + "</td>"
                    "</tr>"

                    "<tr><th></th><td>&nbsp;</td></tr>"

                    "<tr>"
                    "<th>NTP Server</th>"
                    "<td>" + _config.ntp.server + "</td>"
                    "</tr>"

                    "<tr><th></th><td>&nbsp;</td></tr>"

                    "<tr>"
                    "<th>MQTT Host</th>"
                    "<td>" + _config.mqtt.server + "</td>"
                    "</tr>"

                    "<tr>"
                    "<th>MQTT Port</th>"
                    "<td>" + _config.mqtt.port + "</td>"
                    "</tr>"

                    "<tr>"
                    "<th>MQTT User</th>"
                    "<td>" + _config.mqtt.user + "</td>"
                    "</tr>"

                    "<tr>"
                    "<th>MQTT Password</th>"
                    "<td>" + _config.mqtt.password + "</td>"
                    "</tr>"

                    "<tr>"
                    "<th>MQTT ClientID</th>"
                    "<td>" + _config.mqtt.clientID + "</td>"
                    "</tr>"

                    "<tr>"
                    "<th>MQTT Topic Prefix</th>"
                    "<td>" + _config.mqtt.topicPrefix + "</td>"
                    "</tr>"

                    "<tr>"
                    "<th>MQTT Topic Announce</th>"
                    "<td>" + _config.mqtt.topicPrefix + MQTT_TOPIC_ANNOUNCE + "</td>"
                    "</tr>"

                    "<tr>"
                    "<th>MQTT Topic Control</th>"
                    "<td>" + _config.mqtt.topicPrefix + MQTT_TOPIC_CONTROL + "</td>"
                    "</tr>"

                    "<tr>"
                    "<th>MQTT Topic Device</th>"
                    "<td>" + _config.mqtt.topicPrefix + MQTT_TOPIC_DEVICE + "</td>"
                    "</tr>"

                    "<tr><th></th><td>&nbsp;</td></tr>"
                    "</table>"

                    "</div>"
                    "<p><form action='/' method='get'><button>Main Menu</button></form><p>"
                    + _html_footer);
  });

  _WebServer.on("/restart", []() {
    _last_request = millis();
    if (!StateCheck(STATE_CONFIGURING) && _config.device.password[0] && !_WebServer.authenticate(HTTP_WEB_USER, _config.device.password))
      return _WebServer.requestAuthentication();

    _WebServer.send(200, "text/html",
                    _html_header +
                    "<div class='msg'>"
                    "Device will restart now."
                    "</div>"
                    "<p><form action='/' method='get'><button>Main Menu</button></form><p>"
                    + _html_footer);

    /*
        trigger reboot
    */
    StateChange(STATE_WAIT_BEFORE_REBOOTING);
  });

  _WebServer.begin();
  _last_request = millis();
  LogMsg("HTTP: server started");
}

/*
**	handle incoming HTTP requests
*/
void HttpUpdate(void)
{
  _WebServer.handleClient();
}

/*
  return the time in seconds since the last HTTP request
*/
int HttpLastRequest(void)
{
  return (millis() - _last_request) / 1000;
}/**/
