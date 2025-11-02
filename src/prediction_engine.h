#ifndef PREDICTION_ENGINE_H
#define PREDICTION_ENGINE_H

#include "config.h"

class PredictionEngine {
public:
  int predict(float temperatura, float humedad, float presion, float tendenciaHumedad, float tendenciaPresion) {
    int nivelAlerta = 0;
    int puntosRiesgo = 0;

    // SISTEMA DE PUNTOS MULTIVARIABLE
    // Factor 1: Humedad alta
    if (humedad > HUMEDAD_ALERTA_ROJA) puntosRiesgo += 3;
    else if (humedad > HUMEDAD_ALERTA_AMARILLA) puntosRiesgo += 2;
    else if (humedad > 65) puntosRiesgo += 1;

    // Factor 2: Presion baja
    if (presion < PRESION_BAJA_ALERTA) puntosRiesgo += 3;
    else if (presion < PRESION_BAJA_ADVERTENCIA) puntosRiesgo += 2;
    else if (presion < 1015) puntosRiesgo += 1;

    // Factor 3: Tendencia de humedad creciente
    if (tendenciaHumedad > TENDENCIA_HUMEDAD_ALERTA) puntosRiesgo += 2;
    else if (tendenciaHumedad > TENDENCIA_HUMEDAD_ADVERTENCIA) puntosRiesgo += 1;

    // Factor 4: Tendencia de presion decreciente (MUY IMPORTANTE)
    if (tendenciaPresion < TENDENCIA_PRESION_ALERTA) puntosRiesgo += 3;
    else if (tendenciaPresion < -0.1) puntosRiesgo += 2;

    // Factor 5: Temperatura estable o descendiendo
    if (temperatura < 25) puntosRiesgo += 1;

    // EVALUACION FINAL
    if (puntosRiesgo >= 8 || (humedad > 90 && presion < 1010)) {
      nivelAlerta = 2;
      Serial.println("PREDICCION: ALERTA ROJA - Lluvia inminente");
    }
    else if (puntosRiesgo >= 5) {
      nivelAlerta = 1;
      Serial.println("PREDICCION: ALERTA AMARILLA - Posible lluvia");
    }
    else {
      nivelAlerta = 0;
      Serial.println("PREDICCION: NORMAL - Condiciones estables");
    }

    Serial.print("   Puntos riesgo: ");
    Serial.println(puntosRiesgo);
    Serial.print("   Tendencia humedad: ");
    Serial.println(tendenciaHumedad, 3);
    Serial.print("   Tendencia presion: ");
    Serial.println(tendenciaPresion, 3);

    return nivelAlerta;
  }
};

#endif