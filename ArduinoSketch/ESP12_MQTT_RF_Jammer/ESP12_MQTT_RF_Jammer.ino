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

// include ESP8266 related libraries
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>

// include MQTT protocol library. https://github.com/knolleary/pubsubclient
#include <PubSubClient.h>

// include AD9851 library, from https://github.com/handiko/AD9851
#include <AD9851.h>

/*  define the I/O ports that being used
 *  
 *  | DDS pins | ESP12 pins|
 *  |----------|-----------|
 *  | RST      |  GPIO-13  |
 *  | DATA     |  GPIO-12  |
 *  | FQ       |  GPIO-14  |
 *  | CLK      |  GPIO-16  |
 *  
 */
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

// min_freq : the frequency which the Jammer starts transmitting on
// max_freq : the frequency which the Jammer stops transmitting on
unsigned long min_freq = 34800000UL;
unsigned long max_freq = 35200000UL;

void setupWiFi()
{
  delay(100);

  // connect to WiFi(s) using credentials
  wifiMulti.addAP("ssid_from_AP_1", "your_password_for_AP_1");
  wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
  wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

  Serial.println("Connecting ...");

  // during connecting phase, we turn-on the LED
  digitalWrite(LED_BUILTIN, LOW);

  while (wifiMulti.run() != WL_CONNECTED) 
  {
    delay(250);
    Serial.print('.');
  }

  // once sucessfully connected to WiFi, we turn-off the LED
  digitalWrite(LED_BUILTIN, HIGH);

  // print the WiFi info
  Serial.println('\n');
  Serial.print("Connected to:\t");
  Serial.println(WiFi.SSID());
  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
}

/*
 * This function is used to reconnect to the MQTT server
 */
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

      /* 
       * we subscribes into three "inTopic"(s) at once 
       * 
       * inTopicEnable  : Enable/disable the RF Jamming routine
       * inTopicMaxFreq : Sets the max_freq value
       * inTopicMinFreq : Sets the min_freq value
       */
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

/*
 * This function is where the desired actions executed upon the incoming
 * messages from the subscribed topics.
 */
void callback(char* topic, byte* payload, unsigned int len)
{
  char buff[50] = {};

  // print some incoming messages
  Serial.print("Received messages: ");
  Serial.println(topic);
  
  for(unsigned int i=0; i<len; i++)
  {
    Serial.print((char) payload[i]);
    
    buff[i] = (char) payload[i];
  }

  String str((char*)buff);

  /* 
   *  if the incoming topic is "/in/en", then enable
   *  or disable the jammer refering to the payload
   */
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

  /* 
   *  if the incoming topic is "/in/maxFreq",
   *  then put the incoming value into max_freq
   */
  else if(strcmp(topic, inTopicMaxFreq) == 0)
  {
    max_freq = strtoul(str.c_str(), NULL, 0);
  }

  /* 
   *  if the incoming topic is "/in/minFreq",
   *  then put the incoming value into min_freq
   */
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

  // if we lose connection to the server, try to reconnect
  if(!client.connected())
  {
    reconnect();
  }

  client.loop();

  // enable/disable the RF Jammer
  if(enDDS)
  {
    digitalWrite(LED_BUILTIN, LOW);

    // this is where the RF Jamming action is done
    writeFreq(dds, random(min_freq, max_freq)); 
  }
  
  else
  {
    digitalWrite(LED_BUILTIN, HIGH);
    dds_reset(dds);
  }

  currentTime = millis();

  /*
   * every 1.5 seconds we reports :
   * 
   *  - RF Jamming status : (enable/disable)
   *  - max_freq value    : (if RF Jamming status is enabled)
   *  - min_freq value    : (if RF Jamming status is enabled)
   */
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

    // print some infos to the seriam monitor
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
