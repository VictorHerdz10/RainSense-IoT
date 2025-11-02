#ifndef SENSOR_CONTROLLER_H
#define SENSOR_CONTROLLER_H

#include <DHT.h>
#include "config.h"

#if !MODO_SIMULACION
DHT dht(DHTPIN, DHTTYPE);
#endif

struct SensorData {
  float temperatura;
  float humedad;
  float presion;
  unsigned long timestamp;
};

class SensorController {
public:
  void begin() {
    #if !MODO_SIMULACION
      dht.begin();
    #endif
  }

  SensorData readSensors() {
    SensorData data;
    data.timestamp = millis();
    
    #ifdef MODO_SIMULACION
      // 🔧 MODO SIMULACIÓN
      data.temperatura = 25.0 + random(-500, 1000) / 100.0;
      data.humedad = 60 + random(-2000, 4000) / 100.0;
      data.presion = 1013.0 + random(-3000, 3000) / 100.0;
      
      Serial.print("🔧 SIMULACIÓN - ");
    #else
      // 🔌 MODO REAL
      data.temperatura = dht.readTemperature();
      data.humedad = dht.readHumidity();
      
      if (isnan(data.temperatura) || isnan(data.humedad)) {
        Serial.println("❌ Error leyendo DHT22 físico");
        data.temperatura = -1;
        data.humedad = -1;
        return data;
      }
      
      data.presion = 1013.0 + random(-1000, 1000) / 100.0;
      Serial.print("🔌 REAL - ");
    #endif

    Serial.print("T:");
    Serial.print(data.temperatura, 1);
    Serial.print("C H:");
    Serial.print(data.humedad, 1);
    Serial.print("% P:");
    Serial.print(data.presion, 1);
    Serial.println("hPa");

    return data;
  }
};

#endif