# Playstation-Lamp

ESP8266-based hack for the [Playstation Lamp](https://www.google.com/search?q=playstation+lamp) to connect to an MQTT service.

## Challenge

There is a [Playstation Lamp](https://www.google.com/search?q=playstation+lamp) available which consist of the classic Playstation icons (triangle, cricle, cross and square).
This lamp has LED in these icons and a small controller in the base bar.

Why not use this lamp to connected it to the smart home via MQTT to get its state as well use it to signal some conditions.


## Solution

I used an [Espressif ESP8266](https://www.espressif.com/en/products/socs/esp8266) device which has WiFi on-board and a lot of GPIO onboard.

The GPIO is capable of PWM on each pin, so this was used to control intensity of the LEDs.

The implemented web frontend is inspired by [Tasmota](https://github.com/arendst/Tasmota).
Configuration, as well as manual operation is supported.

The implementation supports several modes to control the LEDs, which are:

* OFF (all LEDs)
* ON (all LEDs)
* FADE (will fade left to right and back)
* FLASH (will flash all LEDs synchronous)
* BLINK (will blink all LEDs asynchronus)
* FIRE (will flicker all LEDs on a dark level)


## Replacing the original controller by the ESP8266

The originally installed controller has to be removed and all cables can be reused to connect to the ESP8266.

Connect the wires as follows:

* green triangle to GPIO #0 (D3)
* orange circle to GPIO #4 (D2)
* violet cross to GPIO #5 (D1)
* pink square to GPIO #16 (D0)
* button to GPIO #14 (D5)

Change this for different controller boards to fit your needs.


## Prepare the build environment

To intall the ESP8266 device support in the Arduino IDE, do as follows:

* Open the preferences in the Arduino IDE and add the following URLs to the _Additional Boards Manager URLs_ 
  * [http://arduino.esp8266.com/stable/package_esp8266com_index.json](http://arduino.esp8266.com/stable/package_esp8266com_index.json)
* Open the _Boards Manager_ and search for `esp8266`. Install the found library.  
* Under `Tools`
  * select the Board `Generic ESP8266 Board` and your matching variant which was `NodeMCU` in my case. This depends on the board you use.
  * select the hightest `Upload Speed`
  * select the right `CPU Frequency` for your board
  * select the `Flash Frequency`of `80MHz`
  * select the `Partition Scheme` of `No OTA (Large App)`

## Initialization Procedure

Whenever the Playstation Lamp starts and is not able to connect to your WiFi (eg. because of a missing configuration due to a fresh installation), it enters the configuration mode.
In configuration mode the lamp opens an WiFi Access Point with the SSID `PlaystationLamp-AP-XX:XX:XX`. Connect to it with your smartphone or notebook and configure at lease the WiFi settings. Than restart the controller.

To flash, open the sketch, build and upload to a connected ESP8266. Then follow the **Initialization Procedure** above.


## What is in this respository?

### [Playstation-Lamp Sketch](Playstation-Lamp/)

This is the sketch for the ESP8266 micro controller. Use the [ArduinoIDE](https://www.arduino.cc/en/main/software) to compile and upload into the ESP8266 micro controller.

Follow the section **Prepare the build environtment** above, then open the sketch in the Arduino IDE to build and upload to a connected ESP8266.
Then follow the **Initialization Procedure** above.

