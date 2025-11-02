#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <ArduinoJson.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include "config.h"

// DECLARACIONES extern (sin definir aquí)
extern byte mac[];
extern IPAddress ip;
extern EthernetClient ethClient;
extern HttpClient httpClient;

class HttpClientBackend {
public:
  void begin() {
    #if !MODO_SIMULACION
      Ethernet.begin(mac, ip);
      delay(1000);
      Serial.println("Ethernet inicializado");
    #endif
  }

  bool sendData(float temperatura, float humedad, float presion, int alerta) {
    // Crear JSON
    JsonDocument doc;
    doc["sensor_id"] = "ARDUINO_TROPICAL_01";
    doc["timestamp"] = millis();
    doc["temperatura"] = temperatura;
    doc["humedad"] = humedad;
    doc["presion"] = presion;
    doc["alerta"] = alerta;
    doc["modo"] = MODO_SIMULACION ? "simulacion" : "real";

    String jsonString;
    serializeJson(doc, jsonString);

    Serial.println("ENVIANDO AL BACKEND:");
    Serial.println(jsonString);

    #ifdef MODO_SIMULACION
      // EN SIMULACION: Solo mostrar
      Serial.println("(Modo simulacion - envio simulado)");
      return true;
    #else
      // EN MODO REAL: Enviar HTTP real
      return sendHttpRequest(jsonString);
    #endif
  }

private:
  bool sendHttpRequest(String jsonData) {
    Serial.println("Enviando HTTP real...");
    
    httpClient.beginRequest();
    httpClient.post(BACKEND_ENDPOINT);
    httpClient.sendHeader("Content-Type", "application/json");
    httpClient.sendHeader("Content-Length", jsonData.length());
    httpClient.beginBody();
    httpClient.print(jsonData);
    httpClient.endRequest();

    int statusCode = httpClient.responseStatusCode();
    String response = httpClient.responseBody();

    Serial.print("Respuesta HTTP: ");
    Serial.println(statusCode);

    return (statusCode == 200 || statusCode == 201);
  }
};

#endif