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

#include <stdio.h>
#include <string.h>
#include "aoxa.h"
#include "config.h"
#include "mqtt.h"
#include "util.h"

static int _aoxa_led_pin[AOXA_LEDS] = {
  0, //  D3
  4, //  D2
  5, //  D1
  16 //  D0
};
static int _aoxa_button_pin = 14; // D5

static int _aoxa_mode = AOXA_MODE_OFF;
static unsigned long _aoxa_next = 0;

/*
    setup the configuration
*/
void AoxaSetup(void)
{
  LogMsg("AOXA: checking & correting config");

  /*
     check and correct the config
  */
  if (_config.aoxa.default_mode < AOXA_MODE_OFF || _config.aoxa.default_mode >= AOXA_MODE_LAST)
    _config.aoxa.default_mode = AOXA_MODE_OFF;
  if (!_config.aoxa.fade_speed)
    _config.aoxa.fade_speed = AOXA_FADE_SPEED_DEFAULT;
  _config.aoxa.fade_speed = min(max(_config.aoxa.fade_speed, AOXA_FADE_SPEED_MIN), AOXA_FADE_SPEED_MAX);
  if (!_config.aoxa.flash_speed)
    _config.aoxa.flash_speed = AOXA_FLASH_SPEED_DEFAULT;
  _config.aoxa.flash_speed = min(max(_config.aoxa.flash_speed, AOXA_FLASH_SPEED_MIN), AOXA_FLASH_SPEED_MAX);
  if (!_config.aoxa.blink_speed)
    _config.aoxa.blink_speed = AOXA_BLINK_SPEED_DEFAULT;
  _config.aoxa.blink_speed = min(max(_config.aoxa.blink_speed, AOXA_BLINK_SPEED_MIN), AOXA_BLINK_SPEED_MAX);
  if (!_config.aoxa.fire_speed)
    _config.aoxa.fire_speed = AOXA_FIRE_SPEED_DEFAULT;
  _config.aoxa.fire_speed = min(max(_config.aoxa.fire_speed, AOXA_FIRE_SPEED_MIN), AOXA_FIRE_SPEED_MAX);

  /*
     init the pins
  */
  LogMsg("AOXA: configuring pins for output");
  for (int led = 0; led < AOXA_LEDS; led++)
    pinMode(_aoxa_led_pin[led], OUTPUT);

  /*
     init the pins
  */
  LogMsg("AOXA: configuring pins for input");
  pinMode(_aoxa_button_pin, INPUT_PULLUP);

#if DBG
  /*
     switch them on/off
  */
  for (int led = 0; led < AOXA_LEDS; led++) {
    digitalWrite(_aoxa_led_pin[led], HIGH);
    delay(250);
    digitalWrite(_aoxa_led_pin[led], LOW);
    delay(250);
  }
#endif
  AoxaChangeMode(_config.aoxa.default_mode);
}

/*
   cyclic update of the AOXA leds
*/
void AoxaUpdate(void)
{
  unsigned long now = millis();
  static bool button_down = false;

  if (digitalRead(_aoxa_button_pin) == LOW) {
    button_down = true;
  }
  else if (button_down) {
    /*
       the button got released, so choose the next mode
    */
    button_down = false;
    LogMsg("AOXA: button released");
    AoxaNextMode();
  }

  if (now > _aoxa_next) {
    /*
       it might be the time to change the LEDs
    */
    switch (_aoxa_mode) {
      case AOXA_MODE_FADE:
        /*
           fade: back and forth fading

           pos is an virtual spot which moves forth and back (with a higher resolution than the number of LEDs),
           the leds will be set with an an intensity which depends on the distance to that spot
        */
        {
          static int _pos = 0;
          static bool _forward = true;

          if (_forward) {
            if (++_pos >= AOXA_FADE_RANGE) {
              _pos--;
              _forward = !_forward;
            }
          }
          else {
            if (--_pos < 0) {
              _pos++;
              _forward = !_forward;
            }
          }

          for (int led = 0; led < AOXA_LEDS; led++) {
            int delta = (double) abs(_pos - (double) led * AOXA_FADE_RANGE / (AOXA_LEDS - 1)) * 1023.0 / AOXA_FADE_RANGE;
            //int delta = (AOXA_FADE_RANGE - (double) abs(_pos - led * AOXA_FADE_RANGE / AOXA_LEDS)) / AOXA_FADE_RANGE * 1023;
            analogWrite(_aoxa_led_pin[led], delta);
          }
          _aoxa_next = now + _config.aoxa.fade_speed;
        }
        break;
      case AOXA_MODE_FLASH:
        /*
           flash: all LEDs will toggle
        */
        {
          static bool _toggle = false;

          _toggle = !_toggle;
          for (int led = 0; led < AOXA_LEDS; led++)
            digitalWrite(_aoxa_led_pin[led], (_toggle) ? HIGH : LOW);
          _aoxa_next = now + _config.aoxa.flash_speed;
        }
        break;
      case AOXA_MODE_BLINK:
        /*
           blink: in each cycle one random LED will be toggled
        */
        {
          int led = random(AOXA_LEDS);
          digitalWrite(_aoxa_led_pin[led], !digitalRead(_aoxa_led_pin[led]));
          _aoxa_next = now + _config.aoxa.blink_speed;
        }
        break;
      case AOXA_MODE_FIRE:
        /*
           fire: all LEDs will get a different intensity
        */
        {
          for (int led = 0; led < AOXA_LEDS; led++)
            analogWrite(_aoxa_led_pin[led], AOXA_FIRE_LOW + random(AOXA_FIRE_HIGH - AOXA_FIRE_LOW));
          _aoxa_next = now + _config.aoxa.fire_speed;
        }
        break;
    }
  }
}

/*
   get the AOXA mode
*/
int AoxaGetMode(void)
{
  return _aoxa_mode;
}

/*
   set the AOXA mode
*/
void AoxaChangeMode(int mode)
{
  LogMsg("AOXA: changing mode from %d to %d", _aoxa_mode, mode);

  if (mode == _aoxa_mode)
    return;

  if (mode == AOXA_MODE_DEFAULT)
    mode = _config.aoxa.default_mode;

  switch (_aoxa_mode = mode) {
    case AOXA_MODE_ON:
      /*
         switch all LEDs on, and don't schedule any updates
      */
      for (int led = 0; led < AOXA_LEDS; led++)
        digitalWrite(_aoxa_led_pin[led], HIGH);
      _aoxa_next = 0;
      break;
    default:
      /*
         switch all LEDs off, and schedule updates only if we are no in OFF mode
      */
      for (int led = 0; led < AOXA_LEDS; led++)
        digitalWrite(_aoxa_led_pin[led], LOW);
      _aoxa_next = (_aoxa_mode == AOXA_MODE_OFF) ? 0 : millis();
      break;
  }
  MqttPublishControl(String(AoxaLookupMode(_aoxa_mode)));
}

/*
   set the next AOXA mode
*/
void AoxaNextMode(void)
{
  int new_mode = _aoxa_mode + 1;

  if (new_mode > AOXA_MODE_LAST)
    new_mode = AOXA_MODE_OFF;
  AoxaChangeMode(new_mode);
}

/*
   lookup the given mode
*/
const char *AoxaLookupMode(int mode)
{
#define AOXA_MODE_CASE(mode) case AOXA_MODE_ ## mode: return #mode; break;
  switch (mode) {
      AOXA_MODE_CASE(DEFAULT);
      AOXA_MODE_CASE(OFF);
      AOXA_MODE_CASE(ON);
      AOXA_MODE_CASE(FADE);
      AOXA_MODE_CASE(FLASH);
      AOXA_MODE_CASE(BLINK);
      AOXA_MODE_CASE(FIRE);
  }
  return NULL;
}/**/
