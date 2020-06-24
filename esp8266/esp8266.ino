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

// For BEAR SSL
#include <ESP8266HTTPClient.h>

// HTTPS client
#include <WiFiClientSecure.h>

// HTTP client
#include <HttpClient.h>

#include "config.h"

// to get getVcc() to work, has the doc say :
// http://arduino.esp8266.com/Arduino/versions/2.0.0-rc2/doc/libraries.html#esp-specific-apis
ADC_MODE(ADC_VCC);

// Infrared port
const int IR_LED = 4;   // D2
const int IR_POWER = 5; // D1

// interval between two checkings, in seconds
//const int interval = 60 * 10;
const int interval = 10;

void setup()
{
  Serial.begin(57600);
  SPIFFS.begin();

  // The infrared module supply is wired in a PIN
  // instead of the 3.3V, so it is OFF when the
  // ESP12 is in deepsleep mode.
  // So we have explicitly to turn on the PIN to
  // give power to the infrared module.
  pinMode(IR_POWER, OUTPUT);
  digitalWrite(IR_POWER, HIGH);
  delay(10);

  // prev & current states
  pinMode(IR_LED, INPUT_PULLUP);
  bool state = !digitalRead(IR_LED);
  bool prev_state = load_state();

  if (!prev_state && state) // You have new mail !
  {
    // Wifi
    wifi_connect();

    if (bear_https_free_smsapi(String("You have mail ! (batt:") + ESP.getVcc() + ")"))
    {
      save_state(state);
      Serial.println("You have mail !");
      // http_debug("You have mail");
    }
    else
    {
      Serial.println("You have mail, but SMS notification failed ! Retrying next time...");
    }
  }
  else
  {
    Serial.println(String("Mailbox is ") + (state ? "full" : "empty"));
    // http_debug(String("Mailbox is ") + (state ? "full" : "empty"));

    save_state(state);
  }

  // Sleep
  ESP.deepSleep(interval * 1000000);
  //delay(interval * 1000);
}

void loop()
{
}

void wifi_connect()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    Serial.print(WiFi.status());
  }
}

bool bear_https_free_smsapi(String message)
{
  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  String url = String("/sendmsg?user=") + FREE_USER + "&pass=" + FREE_PASS + "&msg=" + message;

  if (https.begin(client, "smsapi.free-mobile.fr", 443, url))
  {
    int httpsCode = https.GET();
    if (httpsCode > 0)
    {
      Serial.println(httpsCode);
      if (httpsCode == HTTP_CODE_OK)
      {
        Serial.println(https.getString());
        return true;
      }
    }
    else
    {
      Serial.print("failed to GET");
      return false;
    }
  }
  else
  {
    Serial.print("failed to connect to server");
    return false;
  }
}


bool https_free_smsapi(String message)
{
  WiFiClientSecure client;

  // SHA-1 fingerprint
  //const char *fingerprint = "3B 5C 64 35 F5 28 BF 1C FA 96 DC E7 65 B1 E1 B3 2E 84 35 77";
  // 2020-06
  const char *fingerprint = "EB 49 4D F9 6A 8A 87 FC 7D D0 CA 7E 99 20 15 47 37 FC AD 4E";

  client.setFingerprint(fingerprint);

  if (!client.connect("smsapi.free-mobile.fr", 443))
  {
    Serial.println("connection failed");
    return false;
  }

  if (!client.verify(fingerprint, "smsapi.free-mobile.fr"))
  {
    Serial.println("certificate doesn't match");
    return false;
  }

  client.print(String("GET /sendmsg?user=") + FREE_USER + "&pass=" + FREE_PASS + "&msg=" + message);
  client.println(" HTTP/1.1");
  client.println("Host: smsapi.free-mobile.fr");
  client.println();

  return true;
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

void save_state(bool state)
{
  if (!SPIFFS.exists("/state.txt"))
  {
    SPIFFS.format();
    Serial.println("state.txt  formatted");
  }

  File f = SPIFFS.open("/state.txt", "w");
  f.print(state ? "1" : "0");
  f.close();
}

bool load_state()
{
  String value = "";

  if (!SPIFFS.exists("/state.txt"))
  {
    SPIFFS.format();
    Serial.println("state.txt  formatted");
  }

  File file = SPIFFS.open("/state.txt", "r");

  while (file.available())
  {
    value += char(file.read());
  }

  file.close();

  return value == "1";
}