#include "DHT.h"
#include <AuthClient.h>
#include <MicroGear.h>
#include <MQTTClient.h>
#include <SHA1.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

const char* ssid     = "WIFI_SSID";
const char* password = "WIFI_KEY";

#define APPID       "APPID"
#define GEARKEY     "APPKEY"
#define GEARSECRET  "APPSECRET"
#define SCOPE       ""

#define DHTPIN 16
#define DHTTYPE DHT22

WiFiClient client;
AuthClient *authclient;

DHT dht(DHTPIN, DHTTYPE);
int LDR = A0;

int timer = 0;
MicroGear microgear(client);

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println("Connected to NETPIE...");
  microgear.setName("piesensor");
}

void setup() {
  Serial.begin(115200);

  dht.begin();
  microgear.on(CONNECTED,onConnected);

  Serial.println("Starting...");

  if (WiFi.begin(ssid, password)) {

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    //uncomment the line below if you want to reset token -->
    microgear.resetToken();
    microgear.init(GEARKEY, GEARSECRET, SCOPE);
    microgear.connect(APPID);
  }
}

void loop() {
    if (microgear.connected()) {
      microgear.loop();
      timer=0;
      
      float vhud = dht.readHumidity();
      float vtmp = dht.readTemperature();
      
      char ascii[32];
      int LDRValue = analogRead(LDR); 
      //Serial.println(LDRValue);
      if (isnan(vhud) || isnan(vtmp) || vhud > 100 || vtmp > 100){
          vhud = 0.0;
          vtmp = 0.0;
      }
      
      int humid_value = (vhud - (int)vhud) * 100;
      int tempe_value = (vtmp - (int)vtmp) * 100;
      
      if(LDRValue<0){
        LDRValue = 0;
      }else if(LDRValue>600){
        LDRValue = 600;
      }
      int ltg = (LDRValue/60) - (int)(LDRValue/60);
      int light = LDRValue/60;
      if(ltg>0){
        LDRValue = LDRValue+1;
      }

      sprintf(ascii,"%d.%d,%d.%d,%d,esp8266", (int)vtmp, tempe_value,(int)vhud,humid_value,light);
      //Serial.println(ascii);
      microgear.chat("webapp",ascii);
      delay(1000);
    }else{
      Serial.println("connection lost, reconnect...");
      microgear.connect(APPID);
      delay(timer);
      timer+=100;
    }
    delay(1000);
}
