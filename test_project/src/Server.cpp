#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Server.h"

AsyncWebServer server(80);

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
  server.on("/setWaveform", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Waveform changed");
    request->send(200, "text/plain", "Waveform changed");
  });

  server.begin();
}
