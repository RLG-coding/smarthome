// To add an ip address to the ethernet port: 
//    sudo ip addr add 192.168.1.23/24 dev enp2s0
// Access the page by going to 192.168.1.24.

#include <Adafruit_Sensor.h>
#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>

byte mac[] = { 0xAA, 0xBB, 0xCC, 0x00, 0x00, 0x01 };
IPAddress ip(192, 168, 1, 24); // address of the web page
EthernetServer server(80);

#define DHTPIN 0
#define LightPIN 0
#define DHTTYPE DHT22

int lghtReading;
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
  while (!Serial) {}
  Serial.println("Booting up server...");
  Ethernet.begin(mac, ip);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield not found.");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  server.begin();
  Serial.println("Server online!");
  Serial.print("Measures can be found at: ");
  Serial.println(Ethernet.localIP());
}



void loop() {
  EthernetClient client = server.available();

  if (client) {
    Serial.println("> new client");
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        
        
        if (c == '\n' && currentLineIsBlank) {
          // measure readings
          float tempReading = dht.readTemperature();
          float humiReading = dht.readHumidity();
          lghtReading = analogRead(LightPIN);

          // HTML page
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println("Refresh: 5");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.print("Temperature: ");
          client.print(tempReading);
          client.print(" *C</br>");          
          client.print("Humidity: ");
          client.print(humiReading);
          client.print("%</br>");
          client.print("Luminosity: ");
          client.print(lghtReading);
          client.print("</br>");
          client.println("</html>");
          break;
        }
        
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
    Serial.println("> client disconnected");
  }
}
