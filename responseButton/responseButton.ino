#include <WiFi.h>
#include <PubSubClient.h>
#include "secretConfig.h"


WiFiClient espClient;
PubSubClient client(espClient);

// LED Pin
#define LED 16
// btn Pin
#define BTN 17



void setup() {
  
  pinMode(LED, OUTPUT);
  pinMode(BTN, INPUT_PULLUP);
  Serial.begin(9600);

  setup_wifi();
  client.setServer(mqttHost, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", flespiToken, flespiToken)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("warning/dennis");
      client.subscribe("alert/dennis");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.println();

  if(String(topic) == "warning/dennis"){
    digitalWrite(LED, HIGH);
  }
  if(String(topic) == "alert/dennis"){
    digitalWrite(LED, LOW);
  }
  
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if(digitalRead(LED) == HIGH){
    if(digitalRead(BTN) == LOW){
      client.publish("response/dennis", "", 1);
      digitalWrite(LED, LOW);
    }
  }
} 
  

