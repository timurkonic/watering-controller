// Watering controller on Arduino Duemilanove
// v.0.3
// Timur Konic <timur.konic@gmail.com>

#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystalRus.h>

// MAC address of the NIC ethernet controller
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// IP settings
byte ip[] = {192, 168, 2, 111};
byte gateway[] = {192, 168, 2, 1};
byte subnet[] = {255, 255, 255, 0};

// TCP port of server, 80 is default for HTTP 
int tcpPort = 80;

EthernetServer server = EthernetServer(tcpPort);

// LCD
LiquidCrystalRus lcd(22, 23, 24, 25, 26, 27);

// Valve count
int valveCount = 4;
int valvePins[] = {46, 48, 42, 44, 38, 40, 34, 36};
int valveStatuses[] = {0, 0, 0, 0};
int valveMoves[] = {0, 0, 0, 0};
unsigned long valveMillis[] = {-1, -1, -1, -1};

void setup() {
  setupPins();
  setupLCD();
  setupEthernet();
}

void setupPins() {
  for (int i=0; i < valveCount; i++) {
    pinMode(valvePins[i * 2], OUTPUT);
    pinMode(valvePins[i * 2 + 1], OUTPUT);
    digitalWrite(valvePins[i * 2], HIGH);
    digitalWrite(valvePins[i * 2 + 1], HIGH);
  }
}

void setupLCD() {
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("  ЗАГРУЗКА...");
}

void setupEthernet() {
  Ethernet.begin(mac, ip, gateway, subnet);
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    lcd.clear();
    lcd.print("SHIELD NOT FOUND");
    while (true) {
      delay(1);
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    lcd.clear();
    lcd.print("CABLE NOT CONNECTED");
    while (true) {
      delay(1);
    }
  }

  server.begin();
  lcd.clear();
  lcd.print("КОНТРОЛЛЕР ОК");
  lcd.setCursor(0, 1);
  lcd.print(Ethernet.localIP());
  lcd.print(":");
  lcd.print(tcpPort);

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
      delay(1);
    }
  }
  delay(1);
}

void yield() {
  checkTimeouts();
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
  if (uri.startsWith("/cmd=") && uri.length() >= 7) {
    char cValveNumber = uri.charAt(5);
    char cValveStatus = uri.charAt(6);
    int valveNumber = cValveNumber - '1';
    int valveStatus = cValveStatus - '0';
    if (valveNumber >= 0 && valveNumber < valveCount && valveStatus >= 0 && valveStatus <= 1) {
      valveStatuses[valveNumber] = valveStatus;
      valveMoves[valveNumber] = 1;
      valveMillis[valveNumber] = millis();
      setValvePins();
      return true;
    }
  }
  return false;
}

void checkTimeouts() {
  for (int i=0; i < valveCount; i++) {
    if (valveMoves[i] != 0 && (valveMillis[i] > millis() || valveMillis[i] + 30000L <= millis())) {
      valveMoves[i] = 0;
      showValveStatus(i, valveStatuses[i]);
      setValvePins();
    }
  }
}

void setValvePins() {
  for (int i=0; i < valveCount; i++) {
    if (valveMoves[i] != 0) {
      if (valveStatuses[i] == 0) {
        digitalWrite(valvePins[i * 2], HIGH);
      }
      else {
        digitalWrite(valvePins[i * 2], LOW);
      }
      digitalWrite(valvePins[i * 2 + 1], LOW);
    }
    else {
      digitalWrite(valvePins[i * 2], HIGH);
      digitalWrite(valvePins[i * 2 + 1], HIGH);
    }
  }
}

void showValveStatus(int valveNumber, int valveStatus) {
  lcd.clear();
  lcd.print("КРАН ");
  lcd.print(valveNumber + 1);
  lcd.print(valveStatus == 0 ? " ЗАКРЫТ" : " ОТКРЫТ");
}
