#include <Arduino.h>
#include <HX711.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>
#include <EEPROM.h>

#define EEPROM_SIZE 512

// Netzwerkkonfiguration
const char* ssid = "ESP32";
const char* password = "123456789";
const String mdns = "esp32";
IPAddress local_ip(192, 168, 5, 1);
IPAddress gateway(192, 168, 5, 1);
IPAddress subnet(255, 255, 255, 0);

// Server-Konfiguration
const int HTTP_PORT = 80;
const int WS_PORT = 81;
WebServer server(HTTP_PORT);
WebSocketsServer webSocket = WebSocketsServer(WS_PORT);

// Timing
const unsigned long weightSendInterval = 100;  // Intervall in Millisekunden
unsigned long lastWeightSendTime = 0;

// Load Cell Pins
const int LOADCELL_DOUT_PIN = 33;
const int LOADCELL_SCK_PIN = 32;

float calibrationFactor;
float weight;
HX711 scale;

// Forward declarations
void handleRoot();
void handleWebRequests();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void processWebSocketMessage(uint8_t num, const String& message);
void sendWeight();
bool loadFromSpiffs(String path);

void setup() {
    Serial.begin(115200);
    SPIFFS.begin();
    EEPROM.begin(EEPROM_SIZE);
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    // Initialize mDNS
    if (!MDNS.begin(mdns)) {
        Serial.println("Error setting up MDNS responder!");
        return;  // Statt einer Endlosschleife, zurückkehren.
    }

    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

    EEPROM.get(0, calibrationFactor);
    scale.set_scale(calibrationFactor);
    if (calibrationFactor < 1) {
        calibrationFactor = 1;
    }
    Serial.println("Calibration factor: " + String(scale.get_scale()));

    server.onNotFound(handleWebRequests);
    server.on("/", handleRoot);
    server.begin();

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    server.handleClient();
    webSocket.loop();

    if (millis() - lastWeightSendTime > weightSendInterval) {
        sendWeight();
        webSocket.broadcastTXT("calibrationFactor:" + String(calibrationFactor));
        lastWeightSendTime = millis();
    }
}

void handleRoot() {
    File file = SPIFFS.open("/index.html", "r");
    if (file) {
        server.streamFile(file, "text/html");
        file.close();
    } else {
        Serial.println("Error opening index.html");
        server.send(500, "text/plain", "Error opening index.html");
    }
}

void handleWebRequests() {
    if (loadFromSpiffs(server.uri())) return;
}

bool loadFromSpiffs(String path) {
    Serial.println("Trying to load from SPIFFS: " + path); // Debug-Ausgabe des Pfades
    String dataType = "text/plain";
    if(path.endsWith("/")) path += "index.html";
    if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
    else if(path.endsWith(".html")) dataType = "text/html";
    else if(path.endsWith(".htm")) dataType = "text/html";
    else if(path.endsWith(".css")) dataType = "text/css";
    else if(path.endsWith(".js")) dataType = "application/javascript";
    else if(path.endsWith(".coffee")) dataType = "application/coffee";
    else if(path.endsWith(".png")) dataType = "image/png";
    else if(path.endsWith(".gif")) dataType = "image/gif";
    else if(path.endsWith(".jpg")) dataType = "image/jpeg";
    else if(path.endsWith(".ico")) dataType = "image/x-icon";
    else if(path.endsWith(".xml")) dataType = "text/xml";
    else if(path.endsWith(".pdf")) dataType = "application/pdf";
    else if(path.endsWith(".zip")) dataType = "application/zip";
    File dataFile = SPIFFS.open(path.c_str(), "r");
    if (!dataFile) {
        Serial.println("Failed to open file for reading: " + path); // Zeigt eine Fehlermeldung, wenn die Datei nicht geöffnet werden konnte
        return false;
    }
    Serial.println("Successfully opened: " + path); // Bestätigung, dass die Datei geöffnet wurde
    if(server.hasArg("download")) dataType = "application/octet-stream";
    size_t sent = server.streamFile(dataFile, dataType);
    dataFile.close();
    return sent == dataFile.size();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    if (type == WStype_TEXT) {
        processWebSocketMessage(num, String((char *)payload));
    }
}

void processWebSocketMessage(uint8_t num, const String& message) {
    if (message.startsWith("setKnownWeight:")) {
        String knownWeightStr = message.substring(message.indexOf(':') + 1);
        float knownWeight = knownWeightStr.toFloat();
        if (knownWeight > 0) {
            float rawValue = scale.get_units(10);
            calibrationFactor = rawValue / knownWeight;
            scale.set_scale(calibrationFactor);
            Serial.println("New calibration factor set: " + String(calibrationFactor));
            webSocket.sendTXT(num, "Calibration factor updated to " + String(calibrationFactor));
            webSocket.broadcastTXT("calibrationFactor:" + String(calibrationFactor));
        } else {
            webSocket.sendTXT(num, "Invalid weight input");
        }
    } else if (message.startsWith("calibrationFactorInput:")) {
        String calibrationFactorInputStr = message.substring(message.indexOf(':') + 1);
        float calibrationFactorInput = calibrationFactorInputStr.toFloat();
         //weight = calibrationFactorInput;
         calibrationFactor = calibrationFactorInput;
         Serial.println(calibrationFactor);
    } else if (message == "reset_scale") {
        calibrationFactor = 1;
        scale.set_scale(calibrationFactor);
        webSocket.broadcastTXT("calibrationFactor:" + String(calibrationFactor));
        scale.tare();  // Tare the scale after reset
        Serial.println("Scale reset");
        webSocket.sendTXT(num, "Scale reset");
    } else if (message == "tare") {
        scale.tare();
        Serial.println("Scale tared");
        webSocket.sendTXT(num, "Scale tared");
    } else if (message == "saveCalibrationFactor") {
        EEPROM.put(0, calibrationFactor);
        EEPROM.commit();
    }
    // Hier können weitere Nachrichten hinzufügen und verarbeiten
}

void sendWeight() {
    if (scale.is_ready()) {
        weight = scale.get_units(5);
        if (weight < 0) {
            weight = weight * -1;
        }
        String weightStr = String(weight);
        webSocket.broadcastTXT(weightStr);
    }
}