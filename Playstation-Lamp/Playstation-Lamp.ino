/*
  Playstation-Lamp

  (c) 2020 Christian.Lorenz@gromeck.de

  main module


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
#include "led.h"
#include "aoxa.h"
#include "wifi.h"
#include "ntp.h"
#include "http.h"
#include "mqtt.h"
#include "state.h"
#include "util.h"

void setup()
{
  /*
     setup serial communication
  */
  Serial.begin(115200);
  Serial.println();
  LogMsg("*** " __TITLE__ " - Version " GIT_VERSION " ***");

  /*
     initialize the basic sub-systems
  */
  LedSetup(LED_MODE_ON);
  StateSetup(STATE_OPERATION);

  if (!ConfigSetup())
    StateChange(STATE_CONFIGURING);

  if (!WifiSetup()) {
    /*
       something wen't wrong -- enter configuration mode
    */
    LogMsg("SETUP: no WIFI connection -- entering configuration mode");
    StateSetup(STATE_CONFIGURING);
    WifiSetup();
  }

  /*
     setup the other sub-systems
  */
  NtpSetup();
  HttpSetup();
  MqttSetup();
  AoxaSetup();
}

void loop()
{
  /*
     do the cyclic updates of the sub systems
  */
  ConfigUpdate();
  LedUpdate();
  WifiUpdate();
  NtpUpdate();
  HttpUpdate();
  MqttUpdate();
  AoxaUpdate();

  /*
     what to do?
  */
  switch (StateUpdate()) {
    case STATE_CONFIGURING:
      /*
         time to configure the device
      */
      LedSetup(LED_MODE_BLINK_FAST);
      /*
         if there is no activity via HTTP, we will reboot

         this is in case where the device has switched by its own
         into the configuration mode.
      */
      if (HttpLastRequest() > STATE_CONFIGURING_TIMEOUT) {
        LogMsg("LOOP: restarting hte device");
        StateChange(STATE_REBOOT);
      }
      break;
    case STATE_REBOOT:
      /*
         time to boot
      */
      LogMsg("LOOP: restarting the device");
      LedSetup(LED_MODE_OFF);
      AoxaChangeMode(AOXA_MODE_OFF);
      ESP.restart();
      break;
  }
}/**/
