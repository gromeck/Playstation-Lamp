/*
  Playstation-Lamp

  (c) 2020 Christian.Lorenz@gromeck.de

  module to handle the configuration


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

#ifndef __AOXA_H__
#define __AOXA_H__ 1

#define AOXA_LEDS                 4

#define AOXA_FADE_SPEED_DEFAULT   2000
#define AOXA_FADE_SPEED_MIN       10
#define AOXA_FADE_SPEED_MAX       10000

#define AOXA_FADE_RANGE           (AOXA_LEDS * 25)

#define AOXA_BLINK_SPEED_DEFAULT  500
#define AOXA_BLINK_SPEED_MIN      10
#define AOXA_BLINK_SPEED_MAX      10000

#define AOXA_FLASH_SPEED_DEFAULT  500
#define AOXA_FLASH_SPEED_MIN      10
#define AOXA_FLASH_SPEED_MAX      10000

#define AOXA_FIRE_SPEED_DEFAULT   50
#define AOXA_FIRE_SPEED_MIN       10
#define AOXA_FIRE_SPEED_MAX       500

#define AOXA_FIRE_LOW             130
#define AOXA_FIRE_HIGH            300

/*
   STATE handling

   if we are in state configuring NTP, MQTT, BLE and BasicAuth are disabled
*/
enum AOXA_MODE {
  AOXA_MODE_FIRST = -1, // helper
  AOXA_MODE_DEFAULT = -1,
  AOXA_MODE_OFF = 0,
  AOXA_MODE_ON,
  AOXA_MODE_FADE,
  AOXA_MODE_FLASH,
  AOXA_MODE_BLINK,
  AOXA_MODE_FIRE,
  AOXA_MODE_LAST_PLUS_ONE, // helper
};
#define AOXA_MODE_LAST  (AOXA_MODE_LAST_PLUS_ONE - 1)

/*
    setup the configuration
*/
void AoxaSetup(void);

/*
   cyclic update of the configuration
*/
void AoxaUpdate(void);

/*
   get the AOXA mode
*/
int AoxaGetMode(void);

/*
   set the AOXA mode
*/
void AoxaChangeMode(int mode);

/*
   set the next AOXA mode
*/
void AoxaNextMode(void);

/*
   lookup the given mode
*/
const char *AoxaLookupMode(int mode);

#endif
