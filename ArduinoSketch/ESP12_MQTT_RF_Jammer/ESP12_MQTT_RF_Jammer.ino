/*
 *  Copyright (C) 2018 - Handiko Gesang - www.github.com/handiko
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>

#include <PubSubClient.h>

#include <AD9851.h>

#define RST   13
#define DATA  12
#define FQ    14
#define CLK   16

DDS dds;

ESP8266WiFiMulti wifiMulti;
WiFiClient espClient;
PubSubClient client(espClient);

const char* brokerUser = "my_broker_username";
const char* brokerPass = "my_broker_password";
const char* broker = "my_broker_server";

const char* outTopicStatus  = "/my_broker_username/out/stat";
const char* outTopicMaxFreq = "/my_broker_username/out/maxFreq";
const char* outTopicMinFreq = "/my_broker_username/out/minFreq";

const char* inTopicEnable   = "/my_broker_username/in/en";
const char* inTopicMaxFreq  = "/my_broker_username/in/maxFreq";
const char* inTopicMinFreq  = "/my_broker_username/in/minFreq";

long currentTime, lastTime;
bool enDDS = false;

unsigned long min_freq = 34800000UL;
unsigned long max_freq = 35200000UL;

void setupWiFi()
{
  delay(100);
  
  wifiMulti.addAP("ssid_from_AP_1", "your_password_for_AP_1");
  wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
  wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

  Serial.println("Connecting ...");

  digitalWrite(LED_BUILTIN, LOW);

  while (wifiMulti.run() != WL_CONNECTED) 
  {
    delay(250);
    Serial.print('.');
  }

  digitalWrite(LED_BUILTIN, HIGH);
  
  Serial.println('\n');
  Serial.print("Connected to:\t");
  Serial.println(WiFi.SSID());
  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  while(!client.connected())
  {
    Serial.print("\nConnecting to ");
    Serial.println(broker);

    if(client.connect("WemosD1", brokerUser, brokerPass))
    {
      Serial.print("\nConnected to ");
      Serial.println(broker);
      
      client.subscribe(inTopicEnable);
      client.subscribe(inTopicMaxFreq);
      client.subscribe(inTopicMinFreq);
    }
    
    else
    {
      Serial.println("Connecting");
      
      delay(2500);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int len)
{
  char buff[50] = {};
  
  Serial.print("Received messages: ");
  Serial.println(topic);
  
  for(unsigned int i=0; i<len; i++)
  {
    Serial.print((char) payload[i]);
    
    buff[i] = (char) payload[i];
  }

  String str((char*)buff);

  if(strcmp(topic, inTopicEnable) == 0)
  {
    if(len == 1 && payload[0] == '1')
    {
      enDDS = true;
    }
    
    else if(len == 1 && payload[0] == '0')
    {
      enDDS = false;
    }
  }

  else if(strcmp(topic, inTopicMaxFreq) == 0)
  {
    max_freq = strtoul(str.c_str(), NULL, 0);
  }

  else if(strcmp(topic, inTopicMinFreq) == 0)
  {
    min_freq = strtoul(str.c_str(), NULL, 0);
  }
  
  Serial.println();
}

void setup()
{
  Serial.begin(115200);
  
  Serial.println(" ");
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);
  Serial.println(" ");
  
  pinMode(LED_BUILTIN, OUTPUT);

  dds = dds_init(RST, DATA, FQ, CLK);
  dds_reset(dds);
  writeFreq(dds, min_freq);
  
  setupWiFi();
  
  client.setServer(broker, 1883);
  client.setCallback(callback);
}

void loop()
{
  char messageEn[50];
  char messageMaxFreq[50];
  char messageMinFreq[50];
  
  if(!client.connected())
  {
    reconnect();
  }

  client.loop();

  if(enDDS)
  {
    digitalWrite(LED_BUILTIN, LOW);
    writeFreq(dds, random(min_freq, max_freq)); 
  }
  
  else
  {
    digitalWrite(LED_BUILTIN, HIGH);
    dds_reset(dds);
  }

  currentTime = millis();
  
  if(currentTime - lastTime > 1500)
  {
    if(enDDS)
    {
      snprintf(messageEn, 75, "Enabled");
      snprintf(messageMaxFreq, 75, "%lu", max_freq);
      snprintf(messageMinFreq, 75, "%lu", min_freq);
    }
    
    else
    {
      snprintf(messageEn, 50, "Disabled");
    }

    Serial.print("Sending messages:\t");
    Serial.println(messageEn);
    
    client.publish(outTopicStatus, messageEn);

    if(enDDS)
    {
      Serial.print("Sending messages:\t");
      Serial.println(messageMaxFreq);

      Serial.print("Sending messages:\t");
      Serial.println(messageMinFreq);

      client.publish(outTopicMaxFreq, messageMaxFreq);
      client.publish(outTopicMinFreq, messageMinFreq); 
    }

    Serial.println();
    
    lastTime = millis();
  }
}
