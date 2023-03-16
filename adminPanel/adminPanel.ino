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

char* alertSubscribe = "adminAlert";

//set up alert queue
QueueArray<char*> queue;


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
      client.subscribe(alertSubscribe);
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
  lcd.clear();
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.println();
  String messageTemp;

  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }

  char* messageCharArr = (char*)messageTemp.c_str();
  Serial.println(messageCharArr);
  queue.enqueue(messageCharArr);
  Serial.println(queue.front());
  //displayQueue();
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

void displayQueue(){
  //lcd.clear();
  if(queue.count() > 0){
    digitalWrite(LED, HIGH);
    lcd.setCursor(0, 0);
    // print message
    //Serial.println((char*)queue.front());
    lcd.print(queue.front());
    lcd.setCursor(0,1);
    lcd.print(queue.count());
  }
  else{
    digitalWrite(LED, LOW);
    lcd.setCursor(0,0);
    lcd.print("All is gucci");
  }
}

void loop() {
  reconnect();

  //displayQueue();
  if (queue.count() > 0){
    lcd.setCursor(0,0);
    queue.enqueue("lmao");
    Serial.println(queue.dequeue());
    char* a = queue.dequeue();
    Serial.println("raw: ");
    Serial.print(a);

    Serial.println("as String: ");
    Serial.print(String(a));

    lcd.print(a);
  }
  
  
  client.loop();
} 
  

