// To add an ip address to the ethernet port: 
//    sudo ip addr add 192.168.1.23/255.255.255.0 dev enp2s0

// To connect to Raspberry from Linux:
//    sudo ip addr add 192.168.23.2/30 dev enp2s0
//    sudo ssh legoff@192.168.23.1
// Send files from Linux to Raspberry:
//    sudo rcp -r Bureau/a.txt legoff@192.168.23.1:/Bureau

// sudo pip install CoAPthon
// sudo pip install Flask

#include <coap.h>
#include <Dhcp.h>
#include <DHT.h>
#include <Dns.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <SPI.h>

#define DHTTYPE DHT22
#define DHTPIN 0
#define LGHPIN 0
DHT dht(DHTPIN, DHTTYPE);

char ssid[] = "Slate";       // network SSID
char pass[] = "12345678";    // network password
int status = WL_IDLE_STATUS;
WiFiServer server(80);

WiFiUDP Udp;
Coap coap(Udp);

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

// ==  CALLBACKS == //
void callback_temp(CoapPacket &packet, IPAddress ip, int port){
  char response[200];
  
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  String message(p);

  // read temperature from DHT sensor
  float t = dht.readTemperature();

  // error handling
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    coap.sendResponse(ip, port, packet.messageid, "Error reading data from DHT22.");
    return;
  }
  
  sprintf(response, "Temperature: %d °C", (int) t);

  if(packet.code==1){ 
    Serial.println("Received GET request for DHT sensor.");
    coap.sendResponse(ip, port, packet.messageid, response);
  }
}

void callback_hum(CoapPacket &packet, IPAddress ip, int port){
  char response[200];
  
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  String message(p);

  // read humidity from DHT sensor
  float h = dht.readHumidity();

  // error handling
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    coap.sendResponse(ip, port, packet.messageid, "Error reading data from DHT22.");
    return;
  }
  
  sprintf(response, "Humidity: %d%%", (int) h);

  if(packet.code==1){ 
    Serial.println("Received GET request for DHT sensor.");
    coap.sendResponse(ip, port, packet.messageid, response);
  }
}

void callback_apptemp(CoapPacket &packet, IPAddress ip, int port){
  char response[200];
  
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  String message(p);

  // read temp and humidity from DHT sensor
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // error handling
  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    coap.sendResponse(ip, port, packet.messageid, "Error reading data from DHT22.");
    return;
  }

  float at = dht.computeHeatIndex(t, h, false);
  
  sprintf(response, "Apparent temperature: %d °C", (int) at);

  if(packet.code==1){ 
    Serial.println("Received GET request for DHT sensor.");
    coap.sendResponse(ip, port, packet.messageid, response);
  }
}

void callback_light(CoapPacket &packet, IPAddress ip, int port){
  char response[200];

  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  String message(p);

  // read light level from light sensors
  int v=analogRead(LGHPIN);
  if(v>450){
     sprintf(response, "light detected!");    
  }
  else{
     sprintf(response, "no light detected");    
  }
  
  if(packet.code==1){ 
    Serial.println("Received GET request for light sensor.");
    coap.sendResponse(ip, port, packet.messageid, response);
  }
}

// ==  SETUP == //
void setup() {
  Serial.begin(9600);
  dht.begin();
  while (!Serial) {}
  Serial.println("Booting up server...");

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv != "1.1.0") {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  server.begin();

  // list of callbacks
  coap.server(callback_temp, "temp");
  coap.server(callback_hum, "hum");
  coap.server(callback_apptemp, "apptemp");
  coap.server(callback_light, "light");
  
  coap.start();
  Serial.println("Server online!");
  printWiFiStatus();
}

void loop() {
  coap.loop();
}
