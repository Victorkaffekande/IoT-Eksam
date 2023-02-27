#include "HX711.h"
#include "data/secretConfig.h"
#include <WiFi.h>
#include <PubSubClient.h>

#define calibration_factor -165000.0 //This value is obtained using the SparkFun_HX711_Calibration sketch

#define DOUT  16
#define CLK  17
#define THRESHOLD 0.1

WiFiClient espClient;
PubSubClient client(espClient);
HX711 scale;

//const float threshold = 0.1;



void setup() {
  Serial.begin(9600);
  Serial.println("HX711 scale demo");

  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0

  setup_wifi();
  client.setServer(mqttHost, 1883);
  //client.setCallback(callback);

  Serial.println("Readings:");

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

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("WeightSensor", flespiToken, flespiToken)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  float reading = scale.get_units();
  if(reading > THRESHOLD){
    String readingToString = String(reading);
    char* readingToChars = (char*)readingToString.c_str();
    client.publish("dennis/bedStatus", readingToChars, 0);
    delay(2000);
  }
  reconnect();
  //Serial.print("Reading: ");
  //Serial.print(reading, 3); //scale.get_units() returns a float
  //Serial.print(" kg"); //You can change this to kg but you'll need to refactor the calibration_factor
  //Serial.println();
}
