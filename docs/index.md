---
# Feel free to add content and custom Front Matter to this file.
# To modify the layout, see https://jekyllrb.com/docs/themes/#overriding-theme-defaults

layout: home
---


# A tracking system to warn you when some stuff are put in your letterbox

This project is made with an IOT part based on ESP8266, and a webserver,
made with NodeJS/Express


## The project

The development board I used is a wemos D1 mini clone (as cheap as 0,60€),
and a simple (0,30€)

- First, rename config.sample.h to config.h and changes values for your needs.
- Then, install the TimeLib library : https://github.com/PaulStoffregen/Time
- Don't forget to install the ESP8266 community board

![Internet Of Things in a letterbox](https://orelab.github.io/letterbox-iot/letterbox-iot.jpg)

![This is done with an ESP8266 chipset](https://orelab.github.io/letterbox-iot/letterbox-iot-esp8266.jpg)

## Roadmap

This first version is a standalone object, installed in my letterbox with an USB
supply. When the postman put something on the box, the object detects it and call 
an API which sends me an SMS, though a simple HTTPS call.

I plan to release a second version, with LIPOs battery, MCP73871 controller and
some solar panels.

I also plan to build a NodeJS server which will be called as a relay between the
letterbox and the SMS API. It will allow me to record the letterbox statuses, and
also save the battery consumption.

## The server

As the object have no real time clock, the server will propose a route with
a date. The object will get it at startup and synchronise with it every day.

An other feature is the ability to record the states of the object : battery
level, status of the letterbox (empty or full), with datetime.

- npm install
- change the port to comply to the config.h value
- npm run start

(Currently, this part of the projet is in standby)
