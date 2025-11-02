#include "sistema_controller.h"
#include "config.h"

// ======================
// DEFINICIONES DE CONFIG BACKEND
// ======================
const char* BACKEND_URL = "http://tu-backend.com";
const char* BACKEND_ENDPOINT = "/api/datos-climaticos";
const int BACKEND_PORT = 80;

// ======================
// VARIABLES GLOBALES
// ======================
FilteredData datosFiltrados;
unsigned long ultimaLectura = 0;
unsigned long ultimoFiltrado = 0;
unsigned long ultimoEnvio = 0;

// ======================
// VARIABLES ETHERNET
// ======================
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 177);
EthernetClient ethClient;
HttpClient httpClient = HttpClient(ethClient, BACKEND_URL, BACKEND_PORT);

// ======================
// INSTANCIAS GLOBALES
// ======================
SensorController sensorController;
DataFilter dataFilter;
PredictionEngine predictionEngine;
HttpClientBackend httpBackend;

// ======================
// IMPLEMENTACIÓN DE FUNCIONES
// ======================
void leerSensores() {
  SensorData datos = sensorController.readSensors();
  
  if (datos.temperatura > 0 && datos.humedad > 0) {
    dataFilter.addData(datos.temperatura, datos.humedad, datos.presion);
  }
}

void filtrarDatos() {
  datosFiltrados = dataFilter.filter();
  
  if (datosFiltrados.humedad > 0) {
    float tendenciaHumedad = dataFilter.calculateHumidityTrend();
    float tendenciaPresion = dataFilter.calculatePressureTrend();
    predictionEngine.predict(datosFiltrados.temperatura, datosFiltrados.humedad, 
                           datosFiltrados.presion, tendenciaHumedad, tendenciaPresion);
  }
}

void enviarAlBackend() {
  if (datosFiltrados.humedad > 0) {
    float tendenciaHumedad = dataFilter.calculateHumidityTrend();
    float tendenciaPresion = dataFilter.calculatePressureTrend();
    int alerta = predictionEngine.predict(datosFiltrados.temperatura, datosFiltrados.humedad, 
                                        datosFiltrados.presion, tendenciaHumedad, tendenciaPresion);
    
    bool exito = httpBackend.sendData(
      datosFiltrados.temperatura,
      datosFiltrados.humedad, 
      datosFiltrados.presion,
      alerta
    );
    
    if (exito) {
      Serial.println("Datos enviados correctamente");
    } else {
      Serial.println("Error enviando datos");
    }
    
    Serial.println("------------------------------------");
  }
}