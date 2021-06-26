/*
  Playstation-Lamp

  (c) 2020 Christian.Lorenz@gromeck.de

  module to handle the MQTT stuff


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

#include "config.h"
#include "aoxa.h"
#include "mqtt.h"
#include "wifi.h"
#include "util.h"

/*
   MQTT context
*/
static PubSubClient *_mqtt;
String _mqtt_topic_tele;
String _mqtt_topic_cmnd;
String _mqtt_topic_stat;
static unsigned long _mqtt_reconnect_wait = 0;

/*
   this handler is called whenever we receive MQTT commands
*/
static void mqtt_handler(char* topic, uint8_t* data, unsigned int len)
{
  LogMsg("MQTT: handler received: topic:%s  data:%p  len:%d", topic, data, len);
  dump("MQTT: handler", data, len);

  /*
     lets see if one mode matches
  */
  for (int aoxa_mode = AOXA_MODE_FIRST; aoxa_mode <= AOXA_MODE_LAST; aoxa_mode++)
    if (!strncasecmp((const char *) data, AoxaLookupMode(aoxa_mode), len))
      AoxaChangeMode(aoxa_mode);

}

/*
   initialize the MQTT context
*/
void MqttSetup(void)
{
  /*
     check and correct the config
  */
  if (!_config.mqtt.port)
    _config.mqtt.port = MQTT_PORT_DEFAULT;
  _config.mqtt.port = min(max(_config.mqtt.port, MQTT_PORT_MIN), MQTT_PORT_MAX);

  if (StateCheck(STATE_CONFIGURING))
    return;

  LogMsg("MQTT: setting up context");

  _mqtt = new PubSubClient(_wifiClient);
  _mqtt->setServer(_config.mqtt.server, _config.mqtt.port);

  _mqtt_topic_tele = MQTT_TOPIC_TELE "/" + String(_config.mqtt.topicPrefix);
  _mqtt_topic_cmnd = MQTT_TOPIC_CMND "/" + String(_config.mqtt.topicPrefix) + "/state";
  _mqtt_topic_stat = MQTT_TOPIC_STAT "/" + String(_config.mqtt.topicPrefix) + "/state";

  DbgMsg("MQTT: _mqtt_topic_stat: %s", _mqtt_topic_stat.c_str());
  DbgMsg("MQTT: _mqtt_topic_cmnd: %s", _mqtt_topic_cmnd.c_str());
  DbgMsg("MQTT: _mqtt_topic_tele: %s", _mqtt_topic_tele.c_str());

  LogMsg("MQTT: context ready");
}

/*
   cyclic update of the MQTT context
*/
void MqttUpdate(void)
{
  if (StateCheck(STATE_CONFIGURING))
    return;

  if (!_mqtt->connected()) {
    if (millis() > _mqtt_reconnect_wait) {
      /*
         connect the MQTT server
      */
      LogMsg("MQTT: reconnecting %s:%s@%s:%d width clientID %s ...", _config.mqtt.user, _config.mqtt.password, _config.mqtt.server, _config.mqtt.port, _config.mqtt.clientID);
      bool connect_status = _mqtt->connect(
                              _config.mqtt.clientID,
                              _config.mqtt.user,
                              _config.mqtt.password,
                              _mqtt_topic_stat.c_str(),
                              2,  // willQoS
                              true,  // willRetain
                              "{ \"state\":\"disconnected\" }");

      DbgMsg("MQTT: connect_status=%d", connect_status);

      if (connect_status) {
        DbgMsg("MQTT: connected");
        /*
           publish our connection state
        */
        DbgMsg("MQTT: publishing telemetry");
        _mqtt->publish((_mqtt_topic_tele + "/state").c_str(), "connected", true);
        _mqtt->publish((_mqtt_topic_tele + "/Wifi_SSId").c_str(), WifiGetSSID().c_str(), true);
        _mqtt->publish((_mqtt_topic_tele + "/IPAddress").c_str(), WifiGetIpAddr().c_str(), true);
        _mqtt->publish((_mqtt_topic_tele + "/Version").c_str(),GIT_VERSION, true);

        // ... and resubscribe
        _mqtt->subscribe(_mqtt_topic_cmnd.c_str());

        // install the handler for subscribed topics
        _mqtt->setCallback(mqtt_handler);
      }
      else {
        /*
           connection failed
        */
        LogMsg("MQTT: connection failed, rc=%d -- trying again in %d seconds", _mqtt->state(), MQTT_WAIT_TO_RECONNECT);
        _mqtt_reconnect_wait = millis() + MQTT_WAIT_TO_RECONNECT * 1000;
      }
    }
  }
  else
    _mqtt->loop();
}

/*
   publish the given message
*/
void MqttPublishStat(String msg)
{
  msg.toLowerCase();
  DbgMsg("MQTT: publishing: %s=%s", _mqtt_topic_stat.c_str(), msg.c_str());

  if (_mqtt)
    _mqtt->publish(_mqtt_topic_stat.c_str(), msg.c_str(), msg.length());
}/**/
