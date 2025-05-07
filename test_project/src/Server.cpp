#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Server.h"

AsyncWebServer server(80);

void setupWebServer() {
  // Connect to Wi-Fi
  const char* ap_ssid = "theremin_server";
  const char* ap_password = "theremin_user_123";  // optional, at least 8 characters

  WiFi.softAP(ap_ssid, ap_password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point started. IP address: ");
  Serial.println(IP);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  // Serve HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // Example handler
  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Toggle received!");
    request->send(200, "text/plain", "Toggled");
  });

  server.begin();
}
