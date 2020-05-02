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

SPIFFS
https://byfeel.info/eeprom-ou-spiffs/

*/

// HTTP server
#include <ESP8266WiFi.h>

// HTTPS client
#include <WiFiClientSecure.h>

// HTTP client
#include <HttpClient.h>

// https://github.com/PaulStoffregen/Time
// https://projetsdiy.fr/esp8266-web-serveur-partie3-heure-internet-ntp-ntpclientlib/
#include <TimeLib.h>
#include <NtpClientLib.h>

#include "config.h"

// Infrared port
const int IR_LED = 4;

// interval between two checkings, in seconds
const int interval = 60 * 10;

void setup()
{
  Serial.begin(57600);

  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Sync time
  // NTP.onNTPSyncEvent([](NTPSyncEvent_t error) {
  //   if (error) {
  //     Serial.print("Time Sync error: ");
  //     if (error == noResponse)
  //       Serial.println("NTP server not reachable");
  //     else if (error == invalidAddress)
  //       Serial.println("Invalid NTP server address");
  //     else
  //       Serial.println(String("NTP error n°") + error);
  //   } else {
  //     Serial.print("Got NTP time: ");
  //     Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
  //   }
  // });
  // NTP.setInterval(60*60*12); // sync time every 12H
  // NTP.begin("fr.pool.ntp.org", 1, true);

  // infrared
  pinMode(IR_LED, INPUT_PULLUP);

  // SPIFFS
  SPIFFS.begin();
}

void loop()
{
  check_status();

  // every 10 seconds
  // delay(1000*10);

  // every 10 seconds
  // Serial.println("Hello !");
  // http_debug("Hello !");

  ESP.deepSleep(10 * 1000000);
}

void https_free_smsapi(String message)
{
  WiFiClientSecure client;

  const char *fingerprint = "3B 5C 64 35 F5 28 BF 1C FA 96 DC E7 65 B1 E1 B3 2E 84 35 77";

  client.setFingerprint(fingerprint);

  if (!client.connect("smsapi.free-mobile.fr", 443))
  {
    Serial.println("connection failed");
    return;
  }

  if (!client.verify(fingerprint, "smsapi.free-mobile.fr"))
  {
    Serial.println("certificate doesn't match");
    return;
  }

  client.print(String("GET /sendmsg?user=") + FREE_USER + "&pass=" + FREE_PASS + "&msg=" + message);
  client.println(" HTTP/1.1");
  client.println("Host: smsapi.free-mobile.fr");
  client.println();
}

void http_debug(String message)
{
  WiFiClient client;
  client.connect("192.168.1.30", 3000);
  client.println(String("GET /") + urlencode(message) + " HTTP/1.1\n\n");
  client.stop();
}

String urlencode(String str)
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++)
  {
    c = str.charAt(i);
    if (c == ' ')
    {
      encodedString += '+';
    }
    else if (isalnum(c))
    {
      encodedString += c;
    }
    else
    {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9)
      {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9)
      {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
      //encodedString+=code2;
    }
    yield();
  }
  return encodedString;
}

void check_status()
{
  //Serial.print( NTP.getTimeDateString() + " " );

  // Getting the state (true=full)
  bool state = !digitalRead(IR_LED);
  bool prev_state = load_state();

  // If the infrared is ON during the test, an SMS is sent
  if ( ! prev_state && state)
  {
    Serial.println("You have mail !");
    https_free_smsapi("You have mail !");
    // http_debug("You have mail");
  }
  else
  {
    Serial.println(String("Mailbox is ") + (state ? "full" : "empty"));
    // http_debug(String("Mailbox is ") + (state ? "full" : "empty"));
  }

  save_state(state);
}

void save_state(bool state)
{
  if ( ! SPIFFS.exists("/state.txt")) {
    SPIFFS.format();
    Serial.println("state.txt  formatted");
  }

  File f = SPIFFS.open("/state.txt", "w");
  f.print(state?"1":"0");
  f.close();
}

bool load_state()
{
  String value = "";

  if ( ! SPIFFS.exists("/state.txt")) {
    SPIFFS.format();
    Serial.println("state.txt  formatted");
  }

  File file = SPIFFS.open("/state.txt", "r");

  while (file.available())
  {
    value += char(file.read());
  }

  file.close();

  return value=="1";
}