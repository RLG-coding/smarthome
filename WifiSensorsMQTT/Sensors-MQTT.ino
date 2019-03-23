// To connect to Raspberry from Linux:
//    sudo ip addr add 192.168.23.2/30 dev enp2s0
//    sudo ssh legoff@192.168.23.1

#include <DHT.h>
#include <MQTT.h>
#include <SPI.h>
#include <WiFi101.h>
#include <PubSubClient.h>

#define DHTTYPE DHT22
#define DHTPIN 0
#define LGHPIN 0
DHT dht(DHTPIN, DHTTYPE);

const char ssid[] = "Slate";
const char pass[] = "12345678";
int status = WL_IDLE_STATUS;

IPAddress broker(192,168,43,212);
char brokerIP[] =  "192.168.43.212";
char message[100];

WiFiClient net;
PubSubClient client(net);

unsigned long lastMillis = 0;

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println("connected");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
    Serial.println();
}

// connect to Wi-FI then to the MQTT broker
void connect() {
  // Wi-Fi
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv != "1.1.0") {
    Serial.println("Please upgrade the firmware");
  }
  
  Serial.println("Connecting to WiFi network...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to wifi");
  printWiFiStatus();

  // MQTT
  while (!client.connected()) {
    Serial.print("Connecting to MQTT broker... ");
    if (client.connect("ESP8266Client")) {
      Serial.println("Connected to broker");
    } else {
      Serial.print("Failed to connect to the broker: ");
      Serial.print(client.state());
      Serial.println(" Next attempt in 5 seconds.");
      delay(5000);
    }
  }

  Serial.println("Connection established!");
  Serial.println("-----");
}

// ==  SETUP == //
void setup() {
  Serial.begin(9600);
  dht.begin();
  Serial.println("Booting up server...");
  while (!Serial) {}
  
  client.setServer(brokerIP, 1883);
  client.setCallback(callback);  
  Serial.println("Broker online!");
}

// ==  LOOP == //
void loop() {
  if (!client.connected()) {
    connect();
  }

  // publish every 10 seconds
  if (millis() - lastMillis > 10000) {
    lastMillis = millis();
    
    // measure readings
    int lghtReading = analogRead(LGHPIN);
    float tempReading = dht.readTemperature();
    float humiReading = dht.readHumidity();
    float apparenttmp = dht.computeHeatIndex(tempReading, humiReading, false);

    // print on serial monitor (debug)
    Serial.println("Publishing new set of values:");
    Serial.print("Luminosity: ");
    Serial.println(lghtReading);
    Serial.print("Temperature: ");
    Serial.println(tempReading);
    Serial.print("Humidity: ");
    Serial.println(humiReading);
    Serial.print("Apparent temperature: ");
    Serial.println(apparenttmp);
    Serial.println("-----");

    // send to MQTT broker
    client.publish("/sensor/light", String(lghtReading).c_str(), true);
    client.publish("/sensor/temp", String(tempReading).c_str(), true);
    client.publish("/sensor/humi", String(humiReading).c_str(), true);
    client.publish("/sensor/apptemp", String(apparenttmp).c_str(), true);
  }

  client.loop();
}
