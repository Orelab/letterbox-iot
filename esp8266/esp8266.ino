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


#include "config.h"

// Infrared port
const int IR_LED = 4;

// interval between two checkings, in seconds
const int interval = 60 * 10;

void setup()
{
  Serial.begin(57600);
  SPIFFS.begin();

  // prev & current states
  pinMode(IR_LED, INPUT_PULLUP);
  bool state = !digitalRead(IR_LED);
  bool prev_state = load_state();

  
  if(!prev_state && state) // You have new mail !
  {
    // Wifi
    wifi_connect();

    if( https_free_smsapi(String("You have mail ! (batt:")+ESP.getVcc()+")") )
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
    Serial.println(String("Mailbox is ") + (state?"full":"empty"));
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
  }
}



bool https_free_smsapi(String message)
{
  WiFiClientSecure client;

  const char *fingerprint = "3B 5C 64 35 F5 28 BF 1C FA 96 DC E7 65 B1 E1 B3 2E 84 35 77";

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