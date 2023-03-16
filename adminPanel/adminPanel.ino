#include <WiFi.h>
#include <PubSubClient.h>
#include "data/secretConfig.h"
#include <LiquidCrystal_I2C.h>
#include <QueueArray.h>

// LED Pin
#define LED 17
// btn Pin
#define RED_BTN 16
#define BLUE_BTN 4

int lcdColumns = 16;
int lcdRows = 2;

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
  Serial.print("init");
  lcd.backlight();
}

void loop() {
  reconnect();


  //displayQueue();
 
  
  
  client.loop();
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
char *token;
const char s[2] = ",";
void displayAlert(String msg){
  lcd.clear();
  lcd.setCursor(0, 0);
  Serial.println("full msg: " + msg);
  int i1 = msg.indexOf(',');
  int i2 = msg.indexOf(',', i1+1);
  int i3 = msg.indexOf(',', i2+1);


  String id = msg.substring(0, i1);
  String info = msg.substring(i1 + 1, i2);
  String room = msg.substring(i2 + 1, i3);
  String queueLength = msg.substring(i3 + 1, msg.length());

  Serial.println(id);
  Serial.println(info);
  Serial.println(room);
  Serial.println(queueLength);

  lcd.print(info+" "+room);
  lcd.setCursor(0, 1);
  lcd.print("Queue length: " + queueLength);
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
void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i=0; i < lcdColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}



  

