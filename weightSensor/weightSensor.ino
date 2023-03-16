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

String residentID = "1";

String bedtimeSubscribe = "bedtime/" + residentID;
char* bedtimeAsCharArr = (char*)bedtimeSubscribe.c_str();

String responseSubscribe = "response/" + residentID;
char* responseAsCharArr = (char*)responseSubscribe.c_str();

String bedStatusPublish = "bedStatus/" + residentID;
char* bedStatusAsCharArr = (char*)bedStatusPublish.c_str();

String warningPublish = "warning/" + residentID;
char* warningeAsCharArr = (char*)warningPublish.c_str();

String alertPublish = "alert/" + residentID;
char* alertAsCharArr = (char*)alertPublish.c_str();

WiFiClient espClient;
PubSubClient client(espClient);
HX711 scale;

//timer
hw_timer_t * timer = NULL;

bool active = false;
bool inBed = false;
bool firstTimeInBed = true;

bool alert = false;
bool warning = false;
bool response = false;


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

  if(String(topic) == bedtimeAsCharArr){
    if(messageTemp == "activate"){
      active = true;
    }
    if(messageTemp == "deactivate"){
        active = false;
        publishBedStatus("1");
        firstTimeInBed = true;
        inBed = false;
        alert = false;
        warning = false;
        response = false;
    }
  }
  if(String(topic) == responseAsCharArr){
    response = true;
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
      client.subscribe(bedtimeAsCharArr);
      client.subscribe(responseAsCharArr); //TODO make dynamic
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
  String message = String(timerString + "," + status);
  char* timerChars = (char*)message.c_str();
  client.publish(bedStatusAsCharArr, timerChars, 0);
  timerRestart(timer);
}

void resetInterrupt(hw_timer_t * timer, void (*fn)(), int lengthInSeconds){
  double currentTime = timerReadSeconds(timer);
  long lengthInMicroseconds = (lengthInSeconds + currentTime) * 1000000;

  timerDetachInterrupt(timer);
  timerAttachInterrupt(timer, &(*fn), true);

  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timer, lengthInMicroseconds, false);

  // Start an alarm
  timerAlarmEnable(timer);
}

void ARDUINO_ISR_ATTR warningFlag(){
  if(inBed == false && firstTimeInBed == false){
    warning = true;
  }
}

void ARDUINO_ISR_ATTR alertFlag(){
  alert = true;
}

void loop() {
  reconnect();
  if(active){
    float reading = scale.get_units();
    if(reading > THRESHOLD && inBed == false){
      if(firstTimeInBed == false){
        publishBedStatus("0");
      }
      inBed = true;
      firstTimeInBed = false;
      timer = timerBegin(0, 80, true);
    }
    if(reading < THRESHOLD && inBed == true){
      inBed = false;
      publishBedStatus("1");
      timer = timerBegin(0, 80, true);
      resetInterrupt(timer, &warningFlag, 10);
    }
    if(warning){
      client.publish(warningeAsCharArr, "", 0);
      warning = false;
      alert = false;
      response = false;
      resetInterrupt(timer, &alertFlag, 10);
    }
    if(alert && !response){
      alert = false;
      client.publish(alertAsCharArr, "", 0);
    }
  }
  client.loop();
}
