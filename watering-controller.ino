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

// Valve count
int valveCount = 4;


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

String parseRequest(String line) {
  if (line.startsWith("GET ")) {
    int spaceIndex = line.indexOf(' ', 4);
    if (spaceIndex > 0) {
      return line.substring(4, spaceIndex);
    }
  }
  return "";
}

void sendResponse(EthernetClient client, String uri) {
  if (runUri(uri)) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("OK");
  }
  else {
    client.println("HTTP/1.1 404 Not found");
    client.println();
    client.println("Not found");
  }
  delay(1);
  client.stop();
}

boolean runUri(String uri) {
  Serial.print("URI: ");
  Serial.println(uri);
  if (uri.startsWith("/cmd=") && uri.length() >= 7) {
    char cValveNumber = uri.charAt(5);
    char cValveStatus = uri.charAt(6);
    int valveNumber = cValveNumber - '0';
    int valveStatus = cValveStatus - '0';
    if (valveNumber >= 0 && valveNumber < valveCount && valveStatus >= 0 && valveStatus <= 1) {
      Serial.print("Set valve ");
      Serial.print(valveNumber);
      Serial.print(" to ");
      Serial.println(valveStatus);
      return true;
    }
  }
  return false;
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    String requestLine = "";
    String requestUri = "";
    boolean requestLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char requestChar = client.read();
        if (requestChar == '\n' && requestLineIsBlank) {
          sendResponse(client, requestUri);
          break;
        }
        if (requestChar == '\n') {
          requestLineIsBlank = true;
          String parseResult = parseRequest(requestLine);
          if (parseResult.length() > 0) {
            requestUri = parseResult;
          }
          requestLine = "";
        }
        else {
          if (requestChar != '\r') {
            requestLineIsBlank = false;
            if (requestLine.length() < 100) {
              requestLine += requestChar;
            }
          }
        }
      }
    }
  }
}
