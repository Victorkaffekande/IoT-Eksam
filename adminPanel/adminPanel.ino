#include <WiFi.h>
#include <PubSubClient.h>
#include "data/secretConfig.h"
#include <LiquidCrystal_I2C.h>

// LED Pin
#define LED 17
// btn Pin
#define RED_BTN 16
#define BLUE_BTN 4

int lcdColumns = 16;
int lcdRows = 2;

//Alert response id
String alertId = "";

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 

WiFiClient espClient;
PubSubClient client(espClient);

char alertSubscribe[] = "adminAlert";



void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqttHost, 1883);
  client.setCallback(callback);

  //setup buttons and lights
  pinMode(RED_BTN, INPUT_PULLUP);
  pinMode(BLUE_BTN, INPUT_PULLUP);
  pinMode(LED, OUTPUT);

  //initialize LCD
  lcd.init();     
  lcd.backlight();
}

void loop() {
  reconnect();

  if(alertId != ""){
    sendResponse(RED_BTN, "0");
    sendResponse(BLUE_BTN, "1");
  }
  
  client.loop();
} 

void sendResponse(int btn, String value){
  if(digitalRead(btn) == LOW){
     String response = value + String(",") + alertId; 
     char* responseChar = (char*)response.c_str(); 
     alertId = "";
     lcd.clear();
     client.publish("adminResponse", responseChar, 0);
     digitalWrite(LED, LOW);
     delay(2000);
  }
}


void callback(char* topic, byte* message, unsigned int length) {
  lcd.clear();
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.println();
  String messageTemp;

  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }

  
  lcd.print(messageTemp);
  displayAlert(messageTemp);
}

String queueLength;

void displayAlert(String msg){
  lcd.clear();
  lcd.setCursor(0, 0);
  Serial.println("full msg: " + msg);
  int i1 = msg.indexOf(',');
  int i2 = msg.indexOf(',', i1+1);
  int i3 = msg.indexOf(',', i2+1);


  String residentId = msg.substring(0, i1);
  String info = msg.substring(i1 + 1, i2);
  String room = msg.substring(i2 + 1, i3);
  alertId = msg.substring(i3 + 1, msg.length());

  Serial.println(residentId);
  Serial.println(info);
  Serial.println(room);
  Serial.println(alertId);

  lcd.print(info);
  lcd.setCursor(0, 1);
  lcd.print("Room nr: " + room);
  digitalWrite(LED, HIGH);
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
    if (client.connect("adminPanel", flespiToken, flespiToken)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe((char*)alertSubscribe);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}




  

