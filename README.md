.SmartHome.
___
.about.

This repository regroups the different variants of SmartHome, a system designed to connect an interface displayed on a computer or a mobile device to temperature, luminosity and humidity sensors plugged into an Arduino MK100 board.
___
.EthernetSensors.

This system is simple: the Arduino board is directly connected to the computer through an Ethernet cable and the data is transferred thanks to this cable. The interface is a simple HTML page, available at the address 192.168.1.23, displaying the temperature, luminosity and humidity levels in text form.

The Ethernet protocol is simple to implement but requires a physical connection to the device. 
___
.EthernetSensorsCoap.

This system uses the CoAP protocol to decrease the number of messages sent between the Arduino board and the display device. The interface is a terminal window, and the user can access the data by first sending a keyword to the Arduino board (for example: "temp"). This prompts the Arduino board to execute one of four callback functions (tempeartue, apparent temperature, luminosity, humidity), which only then collects the data before sending the measure back.
No messages are sent except when the user asks for them.

The data transits through an Ethernet cable, similarly to the first system.
___
.WifiSensorsCoap.

This variant aims to increase the range of the system by using WiFi to transfer the messages. The interface is, once again, a simple HTML page, generated from a Python script hosted on a Raspberry PI, at the address 192.168.43.212. The Python scripts acts as a proxy - every time the page is accessed, the script sends the four keywords to the Arduino board by using the CoAP protocol through WiFi. The Arduino board and Raspberry PI must be connected to the same access point to achieve this. The board sends the measures back which are displayed on the page.

The browser and the sensors are now connected wirelessly. However, the board Arduino is accessed every time a client connects to the web page. If the data changes slowly or the number of clients is important, this system is inefficient.
___
.WiFiSensorsMQTT.

The last system fixes the previous problem by using the MQTT protocol. The Rasberry PI now acts as a "broker" server, and the Arduino becomes a "publisher" to that server. The Arduino board periodically sends data to the server, and the broker server stores that data. When a client connects to the interface, it "subscribes" to the server. The server will then send updates on the data, to every subscriber, every time such an update is available. The client never interacts directly with the Arduino board.

The interface is a visual dashboard created using NodeRED, hosted on the Ethernet interface of the Raspberry PI.
___
._libraries.

Contains the folders for Arduino libraries used by the different systems.