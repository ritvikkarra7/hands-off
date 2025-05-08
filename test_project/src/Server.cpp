#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Server.h"
#include "WaveFormGenerator.h"

// Reference to the global WaveFormGenerator instance
extern WaveFormGenerator *sampleSource;
AsyncWebServer server(80);

void handleSetWaveform(AsyncWebServerRequest *request) {
  if (request->hasParam("type")) {
    String waveform = request->getParam("type")->value();
    waveform.toLowerCase(); // Convert to lowercase for case-insensitive comparison

    if (waveform == "sine") {
      sampleSource->setWaveType(SINE);
    } else if (waveform == "square") {
      sampleSource->setWaveType(SQUARE);
    } else if (waveform == "triangle") {
      sampleSource->setWaveType(TRIANGLE);
    } else {
      request->send(400, "text/plain", "Invalid waveform type");
      return;
    }

    Serial.println("Waveform changed to: " + waveform);
    request->send(200, "text/plain", "Waveform updated to " + waveform);
  } else {
    request->send(400, "text/plain", "Missing 'type' parameter");
  }
}

void setupWebServer() {
  // Connect to Wi-Fi
  const char* ssid = "karraandfriends";
  const char* password = "agree5013dinner";

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  server.serveStatic("/functions.js", SPIFFS, "/functions.js");
  server.serveStatic("/style.css", SPIFFS, "/style.css");

  // Serve HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // Example handler
  server.on("/setWaveform", HTTP_GET, handleSetWaveform); 

  server.begin();
}
