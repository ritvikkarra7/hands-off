#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Server.h"
#include "WaveFormGenerator.h"

// Reference to the global WaveFormGenerator instance
extern WaveFormGenerator *sampleSource;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // WebSocket server

void handleSetWaveform(AsyncWebServerRequest *request) {
  if (!request->hasParam("type")) {
    Serial.println("Error: Missing 'type' parameter");
    request->send(400, "text/plain", "Missing 'type' parameter");
    return;
  }

  String waveform = request->getParam("type")->value();
  waveform.toLowerCase(); // Convert to lowercase for case-insensitive comparison

  if (waveform == "sine") {
    if (sampleSource) {
      sampleSource->setWaveType(SINE);
      Serial.println("Waveform changed to: sine");
      request->send(200, "text/plain", "Waveform updated to sine");
    } else {
      Serial.println("Error: sampleSource is NULL");
      request->send(500, "text/plain", "Internal Server Error: sampleSource is NULL");
    }
  } else if (waveform == "square") {
    if (sampleSource) {
      sampleSource->setWaveType(SQUARE);
      Serial.println("Waveform changed to: square");
      request->send(200, "text/plain", "Waveform updated to square");
    } else {
      Serial.println("Error: sampleSource is NULL");
      request->send(500, "text/plain", "Internal Server Error: sampleSource is NULL");
    }
  } else if (waveform == "triangle") {
    if (sampleSource) {
      sampleSource->setWaveType(TRIANGLE);
      Serial.println("Waveform changed to: triangle");
      request->send(200, "text/plain", "Waveform updated to triangle");
    } else {
      Serial.println("Error: sampleSource is NULL");
      request->send(500, "text/plain", "Internal Server Error: sampleSource is NULL");
    }
  } else if (waveform == "sawtooth") {
    if (sampleSource) {
      sampleSource->setWaveType(SAWTOOTH);
      Serial.println("Waveform changed to: sawtooth");
      request->send(200, "text/plain", "Waveform updated to sawtooth");
    } else {
      Serial.println("Error: sampleSource is NULL");
      request->send(500, "text/plain", "Internal Server Error: sampleSource is NULL");
    }
  } else {
    Serial.println("Error: Invalid waveform type");
    request->send(400, "text/plain", "Invalid waveform type");
  }
}

void handleSetMode(AsyncWebServerRequest *request)
{
  if (request->hasParam("value")) {
    String mode = request->getParam("value")->value();
    mode.toLowerCase();
    Serial.printf("Received mode: %s\n", mode.c_str());
    if (mode == "digital"){
      digitalWrite(SEL, LOW); // Set SEL low for digital mode
      Serial.println("Digital mode selected");
    }
    else if (mode == "analog"){
      digitalWrite(SEL, HIGH); // Set SEL high for analog mode
      Serial.println("Analog mode selected");
    } else {
      Serial.println("Error: Invalid mode");
      request->send(400, "text/plain", "Invalid mode value");
      return;
    }
    request->send(200, "text/plain", "Mode received: " + mode);
    } else {
    request->send(400, "text/plain", "Missing mode value");
  }
}

void handleSetScale(AsyncWebServerRequest *request)
{
  if (request->hasParam("value")) {
    String scale = request->getParam("value")->value();
    scale.toLowerCase();
    Serial.printf("Received scale: %s\n", scale.c_str());

    if (scale == "default") {
      sampleSource->setScale(SCALE_DEFAULT);
    } else if (scale == "chromatic") {
      sampleSource->setScale(SCALE_CHROMATIC);
    } else if (scale == "major") {
      sampleSource->setScale(SCALE_MAJOR);
    } else if (scale == "minor") {
      sampleSource->setScale(SCALE_MINOR);
    } else if (scale == "pentatonic") {
      sampleSource->setScale(SCALE_PENTATONIC);
    } else {
      request->send(400, "text/plain", "Invalid scale value");
      return;
    }

    request->send(200, "text/plain", "Scale set to: " + scale);
  } else {
    request->send(400, "text/plain", "Missing scale value");
  }
}


// Broadcast frequency to WebSocket clients
void broadcastFrequency(float frequency) {
  // Serial.printf("Broadcasting frequency: %.2f Hz\n", frequency);
  ws.textAll(String(frequency)); // Send the frequency to all connected WebSocket clients
}

void setupWebServer() {
  // Connect to Wi-Fi
  WiFi.softAP("theremin_server", "theremin_user");  // SSID, password
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  server.serveStatic("/functions.js", SPIFFS, "/functions.js");
  server.serveStatic("/styles.css", SPIFFS, "/styles.css");

  // Serve HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // Example handler
  server.on("/setWaveform", HTTP_GET, handleSetWaveform);
  server.on("/setMode", HTTP_GET, handleSetMode); 
  server.on("/setScale", HTTP_GET, handleSetScale); 


  // Add WebSocket handler
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.printf("WebSocket client #%u connected\n", client->id());
    } else if (type == WS_EVT_DISCONNECT) {
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
    }
  });
  server.addHandler(&ws);

  server.begin();
  Serial.println("HTTP server started");
}

void startWebServices() {
  setupWebServer();
}
