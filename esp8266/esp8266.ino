/*

ESP8266
http://framboiseaupotager.blogspot.com/2018/05/tous-les-secrets-de-lesp01-realisation.html
https://www.mobilefish.com/developer/iota/iota_quickguide_esp01s_mam.html
https://f-leb.developpez.com/tutoriels/arduino/esp8266/debuter/#LVI-C

Deep sleep
https://projetsdiy.fr/esp8266-test-mode-deep-sleep-reveil-wakeup-detecteur-mouvement-pir/#Differents_modes_de_veille_et_consommation_d8217un_module_ESP8266EX

Panneau solaire + MCP73871
https://www.bakke.online/index.php/2017/08/06/solar-powered-esp8266/
https://ouiouioui.squarespace.com/?offset=1521548273576

*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>


#include "config.h"


WiFiServer WebServer(80);
WiFiClient client;

const int IR_LED = 2;




void setup() {
  Serial.begin(57600);

  Serial.printf("Wi-Fi mode set to WIFI_STA %s\n", WiFi.mode(WIFI_STA) ? "" : "Failed!");

  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print( wifi_status() );
    Serial.print(".");
  }

//  WiFi.printDiag(Serial);


  // Start the Web Server
  WebServer.begin();
  Serial.println("Web Server started");

  // Print the IP address
  Serial.print("You can connect to the ESP8266 at this URL: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  // Send IP by SMS
  http_request( "Letterbox agent IP is http://" + WiFi.localIP().toString() );

  // infrared
  pinMode(IR_LED, INPUT_PULLUP);
}




void loop() {
  client = WebServer.available();

  if (!client) {
    return;
  }

  while (!client.available()) {
    delay(1);
  }

  String request = client.readStringUntil('\r\n');
  Serial.println(request);
  client.flush();

  if (request.indexOf("/LED=ON") != -1) {}

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html; charset=UTF-8");
  client.println("");
  client.println("<!DOCTYPE HTML><html><head>");
  client.println("<title>Letter box checker</title>");
  client.println("</head><body><p>Letter box is ");
  client.println( digitalRead(IR_LED) ? "empty" : "full" );
  client.println("</p></body></html>");
}





void http_request( String message )
{
  const char* fingerprint = "3B 5C 64 35 F5 28 BF 1C FA 96 DC E7 65 B1 E1 B3 2E 84 35 77";
  WiFiClientSecure client;

  client.setFingerprint(fingerprint);

  if (! client.connect("smsapi.free-mobile.fr", 443)) {
    Serial.println("connection failed");
    return;
  }

  if (! client.verify(fingerprint, "smsapi.free-mobile.fr")) {
    Serial.println("certificate doesn't match");
    return;
  }

  client.print(String("GET /sendmsg?user=") + free_user + "&pass=" + free_pass + "&msg=" + message );
  client.println(" HTTP/1.1");
  client.println("Host: smsapi.free-mobile.fr");
  client.println();
}
