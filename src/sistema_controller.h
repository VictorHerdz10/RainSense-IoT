#ifndef SISTEMA_CONTROLLER_H
#define SISTEMA_CONTROLLER_H

#include "sensor_controller.h"
#include "data_filter.h"
#include "prediction_engine.h"
#include "http_client.h"

// ======================
// DECLARACIONES DE VARIABLES GLOBALES
// ======================
extern SensorController sensorController;
extern DataFilter dataFilter;
extern PredictionEngine predictionEngine;
extern HttpClientBackend httpBackend;
extern FilteredData datosFiltrados;
extern unsigned long ultimaLectura;
extern unsigned long ultimoFiltrado;
extern unsigned long ultimoEnvio;

// ======================
// DECLARACIONES ETHERNET
// ======================
extern byte mac[];
extern IPAddress ip;
extern EthernetClient ethClient;
extern HttpClient httpClient;

// ======================
// DECLARACIONES DE FUNCIONES
// ======================
void leerSensores();
void filtrarDatos();
void enviarAlBackend();

#endif