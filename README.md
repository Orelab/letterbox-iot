# A tracking system to warn you when some stuff are put in your letterbox

This project is made with an IOT part based on ESP8266, and a webserver,
made with NodeJS/Express


## The object

The development board I used is a wemos D1 mini clone (as cheap as 0,60€),
and a simple (0,30€)


## The server

As the object have no real time clock, the server will propose a route with
a date. The object will get it at startup and synchronise with it every day.

An other feature is the ability to record the states of the object : battery
level, status of the letterbox (empty or full), with datetime.


## Planned

I just bough some solar panels, LIPOs and a MCP73871 to control all that,
and make the project autonomous :) I'll share my work here in the future...