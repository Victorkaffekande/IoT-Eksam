#include "HX711.h"
#include "data/secretConfig.h"
#include <WiFi.h>
#include <PubSubClient.h>

#define calibration_factor -165000.0 //This value is obtained using the SparkFun_HX711_Calibration sketch

#define DOUT  16
#define CLK  17
//Weight threshold
#define THRESHOLD 0.1
//10 seconds out of bed until warning
#define TIMELIMIT 10

String resident = "dennis";

WiFiClient espClient;
PubSubClient client(espClient);
HX711 scale;

//timer
hw_timer_t * timer = NULL;
bool active = true;
bool inBed = false;
bool firstTimeInBed = true;

bool warning = false;


void setup() {
  Serial.begin(9600);
  Serial.println("HX711 scale demo");

  //scale
  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0

  //Wifi and mqtt
  setup_wifi();
  client.setServer(mqttHost, 1883);
  client.setCallback(callback);

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

  Serial.print(messageTemp);
  if(String(topic) == "dennis/bedtime"){
    if(messageTemp == "activate"){
      active = true;
    }
    if(messageTemp == "deactivate"){
      active = false;
    }
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("WeightSensor", flespiToken, flespiToken)) {
      Serial.println("connected");
      //subscribe
      client.subscribe("dennis/bedtime"); //TODO make dynamic
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void publishBedStatus(String status){
  uint64_t timerReading = timerReadSeconds(timer);
  String timerString = String(timerReading);
  String message = String("{Time:"+timerString + "," + "Status:" + status + "}");
  char* timerChars = (char*)message.c_str();
  client.publish("dennis/bedStatus", timerChars, 1);
  timerRestart(timer);
}


void ARDUINO_ISR_ATTR warningFlag(){
    if(inBed == false && firstTimeInBed == false){
    warning = true;
    Serial.print(warning);
    Serial.print("WARNING");
    Serial.println();
  }
}

void resetInterrupt(hw_timer_t * timer){
  timerDetachInterrupt(timer);
  timerAttachInterrupt(timer, &warningFlag, true);

  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timer, 10000000, false);

  // Start an alarm
  timerAlarmEnable(timer);
}

void loop() {
  reconnect();
  if(active){
    float reading = scale.get_units();
    if(reading > THRESHOLD && inBed == false){
      if(firstTimeInBed == false){
        publishBedStatus("Out of bed");
      }
      inBed = true;
      firstTimeInBed = false;
      Serial.print("you're in bed");
      timer = timerBegin(0, 80, true);
    }
    if(reading < THRESHOLD && inBed == true){
      inBed = false;
      publishBedStatus("In Bed");
      timer = timerBegin(0, 80, true);
      resetInterrupt(timer);
      Serial.print("you're out of bed");
    }
    if(warning){
      client.publish("warning/dennis", "", 1);
      warning = false;
    }
  }
  client.loop();
}
