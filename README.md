# ESP12 & AD9851 DDS RF Jammer Controlled via MQTT Protocol
This project demonstrate a rather simple implementation of ESP-12 & AD9851 DDS as an RF Jammer (random frequency hopping) which can be controlled via MQTT protocol over the internet.
The parameters which we can control:
* RF Jamming action (enable/disable)
* Starting Frequency (minimum freq.)
* Stop Frequency (maximum freq.)

And then the device then reports back these parameters.

Tested on Wemos D1 mini and cheap AD9851 DDS module with **30MHz external clock**.
Compiled using Arduino IDE 1.8.9.

## Dependencies
There are a few dependencies required:
* [ESP8266 Core for Arduino](https://github.com/esp8266/Arduino). Please follow the installation instruction.
* [PubSubClient](https://github.com/knolleary/pubsubclient) library. Personally, I am using [this](https://github.com/knolleary/pubsubclient) library.
* My [AD9851 DDS](https://github.com/handiko/AD9851) library. Please follow the installation [instruction](https://github.com/handiko/AD9851#instalation).

## Usage

## In Action

## TODO
* Optimize the code (there is a lot of room for improvements!!)
* Adding capabitilies for controlling another I/O ports.
* ...

## Contributing
1. Fork it [https://github.com/handiko/ESP12-MQTT-RF-Jammer/fork](https://github.com/handiko/ESP12-MQTT-RF-Jammer/fork)
2. Create new branch (`git checkout -b myfeature`)
3. Do some editing / create new feature
4. Commit your works (`git commit -m "Adding some feature blah blah blah.."`)
5. Push to the branch (`git push -u origin myfeature`)
6. Create a new Pull Request
