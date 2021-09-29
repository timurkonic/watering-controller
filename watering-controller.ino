// Watering controller on Arduino Duemilanove
// v.0.1
// Timur Konic <timur.konic@gmail.com>

#include <SPI.h>
#include <Ethernet.h>

// MAC address of the NIC ethernet controller
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// IP settings
byte ip[] = {192, 168, 2, 111};
byte gateway[] = {192, 168, 2, 1};
byte subnet[] = {255, 255, 255, 0};

// TCP port of server, 80 is default for HTTP 
int tcpPort = 80;

EthernetServer server = EthernetServer(tcpPort);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Watering controller");

  Ethernet.begin(mac, ip, gateway, subnet);
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
  Serial.print(Ethernet.localIP());
  Serial.print(":");
  Serial.println(tcpPort);
}

void parseURI(String uri) {
  Serial.print("GET ");
  Serial.println(uri);
}

void parseRequest(String line) {
  if (line.startsWith("GET ")) {
    int spaceIndex = line.indexOf(' ', 4);
    if (spaceIndex > 0) {
      String uri = line.substring(4, spaceIndex);
      parseURI(uri);
    }
  }
}

void sendResponse(EthernetClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("OK");
  delay(1);
  client.stop();
}

void loop() {
  EthernetClient client = server.available();
  String requestLine = "";
  if (client) {
    boolean requestLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char requestChar = client.read();
        if (requestChar == '\n' && requestLineIsBlank) {
          sendResponse(client);
          break;
        }
        if (requestChar == '\n') {
          requestLineIsBlank = true;
          parseRequest(requestLine);
          requestLine = "";
        }
        else {
          if (requestChar != '\r') {
            requestLineIsBlank = false;
            requestLine += requestChar;
          }
        }
      }
    }
  }
}
