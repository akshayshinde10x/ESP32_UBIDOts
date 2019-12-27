/****************************************
 * Include Libraries
 ****************************************/
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define WIFISSID "TechAmplifiers" // Put your WifiSSID here
#define PASSWORD "***********" // Put your wifi password here
#define TOKEN "BBFF-**************8p" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "akshay" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
                                           //it should be a random and unique ascii string and different from all other devices

#define DHTPIN 2     // Digital pin connected to the DHT sensor

#define DHTTYPE DHT11   // DHT 11


DHT dht(DHTPIN, DHTTYPE);
/****************************************
 * Define Constants
 ****************************************/
#define VARIABLE_LABEL "temp" // Assing the variable label
#define VARIABLE_LABEL1 "hum"
#define DEVICE_LABEL "esp32" // Assig the device label

char mqttBroker[]  = "industrial.api.ubidots.com";
char payload[100];
char payload1[100];
char topic[150];
// Space to store values to send
char str_sensor[10];
char str_sensor1[10];

/****************************************
 * Auxiliar Functions
 ****************************************/
WiFiClient ubidots;
PubSubClient client(ubidots);

void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  Serial.write(payload, length);
  Serial.println(topic);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    // Attemp to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

/****************************************
 * Main Functions
 ****************************************/
void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFISSID, PASSWORD);
  // Assign the pin as INPUT 
  //pinMode(SENSOR, INPUT);
  dht.begin();
  Serial.println();
  Serial.print("Wait for WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload1, "%s", "");
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL); // Adds the variable label
   sprintf(payload1, "{\"%s\":", VARIABLE_LABEL1); 
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  Serial.println(h);
    Serial.println(t);

  /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
  dtostrf(h, 4, 2, str_sensor);
  dtostrf(t, 4, 2, str_sensor1);
    
  sprintf(payload, "%s {\"value\": %s}}", payload, str_sensor); // Adds the value
  sprintf(payload1, "%s {\"value\": %s}}", payload1, str_sensor1);
  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);
   client.publish(topic, payload1);
  client.loop();
  delay(1000);
}
