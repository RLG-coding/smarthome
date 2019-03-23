// To download coap: 
//    sudo apt-get install libcoap-1-0-bin
// To add an ip address to the ethernet port: 
//    sudo ip addr add 192.168.1.23/255.255.255.0 dev enp2s0
// To run coap: 
//    coap-client -m get coap://192.168.1.1/<callback>

#include <coap.h>
#include <Dhcp.h>
#include <DHT.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>

#define DHTTYPE DHT22
#define DHTPIN 0
#define LGHPIN 0
DHT dht(DHTPIN, DHTTYPE);

IPAddress ip(192, 168, 1, 1); // address of the CoAP server
byte mac[] = { 0xAA, 0xBB, 0xCC, 0x00, 0x00, 0x01 };

EthernetUDP Udp;
Coap coap(Udp);

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

  // read light level from light sensor
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
  Ethernet.begin(mac, ip);

  // list of callbacks
  coap.server(callback_temp, "temp");
  coap.server(callback_hum, "hum");
  coap.server(callback_apptemp, "apptemp");
  coap.server(callback_light, "light");
  
  coap.start();
  
  Serial.println("Server online!");
  Serial.print("Measures can be found at: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  coap.loop();
}
