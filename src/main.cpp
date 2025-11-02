#include <Arduino.h>
#include "config.h"
#include "sistema_controller.h"

void setup() {
  Serial.begin(9600);
  
  Serial.println("====================================");
  #if MODO_SIMULACION
    Serial.println("MODO SIMULACION ACTIVADO");
  #else
    Serial.println("MODO HARDWARE REAL ACTIVADO");
    sensorController.begin();
    httpBackend.begin();
  #endif
  Serial.println("====================================");
}

void loop() {
  unsigned long tiempoActual = millis();
  
  // 1. LECTURA DE SENSORES
  if (tiempoActual - ultimaLectura >= INTERVALO_LECTURA) {
    ultimaLectura = tiempoActual;
    leerSensores();
  }
  
  // 2. FILTRADO DE DATOS
  if (tiempoActual - ultimoFiltrado >= INTERVALO_FILTRADO) {
    ultimoFiltrado = tiempoActual;
    filtrarDatos();
  }
  
  // 3. ENVIO AL BACKEND
  if (tiempoActual - ultimoEnvio >= INTERVALO_ENVIO) {
    ultimoEnvio = tiempoActual;
    enviarAlBackend();
  }
}