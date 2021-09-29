// Watering controller on Arduino Duemilanove
// v.0.1
// Timur Konic <timur.konic@gmail.com>

#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // MAC address of the NIC ethernet controller
IPAddress ip(192, 168, 2, 111); // IP address of the controller
EthernetServer server(80); // IP port of server, 80 is default for HTTP 

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Watering controller");

  Ethernet.begin(mac, ip);
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found");
    while (true) {
      delay(1);
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  server.begin();
  Serial.print("Controller is at ");
  Serial.println(Ethernet.localIP());
}

void loop() {
}
