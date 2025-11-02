#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <vector>
#include <cmath>

using namespace std;

// ======================
// CONFIGURACION
// ======================
const unsigned long INTERVALO_LECTURA = 5000;    // 5 segundos
const unsigned long INTERVALO_FILTRADO = 30000;  // 30 segundos
const unsigned long INTERVALO_ENVIO = 60000;     // 1 minuto

// ======================
// SIMULACION DE ARDUINO
// ======================
unsigned long millis() {
    static auto start = chrono::steady_clock::now();
    auto now = chrono::steady_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(now - start).count();
}

void delay(unsigned long ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
}

class SerialClass {
public:
    void begin(int baud) { 
        cout << "Serial iniciado a " << baud << " baudios" << endl; 
    }
    void println(const string& msg) { cout << msg << endl; }
    void print(const string& msg) { cout << msg; }
    void println(float value) { cout << value << endl; }
    void print(float value) { cout << value; }
    void println(int value) { cout << value << endl; }
    void print(int value) { cout << value; }
};

SerialClass Serial;

// ======================
// ESTRUCTURAS DE DATOS
// ======================
struct SensorData {
    float temperatura;
    float humedad;
    float presion;
    unsigned long timestamp;
};

struct FilteredData {
    float temperatura;
    float humedad;
    float presion;
};

// ======================
// CLASE SensorController (SIMULACION)
// ======================
class SensorController {
public:
    void begin() {
        Serial.println("SensorController inicializado (simulacion)");
    }

    SensorData readSensors() {
        SensorData data;
        data.timestamp = millis();
        
        // Generar datos simulados con patrones realistas para lluvia
        // Simular correlacion entre variables meteorologicas
        float baseHumedad = 40 + (rand() % 6000) / 100.0;  // 40.0 - 100.0%
        
        // Cuando la humedad es alta, la presion tiende a bajar y la temperatura a disminuir
        if (baseHumedad > 80) {
            data.presion = 1000.0 + (rand() % 1500) / 100.0;  // 1000.0 - 1015.0 hPa (baja presion)
            data.temperatura = 18.0 + (rand() % 1500) / 100.0; // 18.0 - 33.0°C (mas fresco)
        } else if (baseHumedad > 60) {
            data.presion = 1010.0 + (rand() % 1500) / 100.0;  // 1010.0 - 1025.0 hPa
            data.temperatura = 22.0 + (rand() % 1800) / 100.0; // 22.0 - 40.0°C
        } else {
            data.presion = 1015.0 + (rand() % 1500) / 100.0;  // 1015.0 - 1030.0 hPa (alta presion)
            data.temperatura = 25.0 + (rand() % 2000) / 100.0; // 25.0 - 45.0°C (mas calido)
        }
        
        data.humedad = max(30.0f, min(100.0f, baseHumedad)); // Limitar entre 30-100%
        
        Serial.print("SIMULACION - T:");
        Serial.print(data.temperatura);
        Serial.print("C H:");
        Serial.print(data.humedad);
        Serial.print("% P:");
        Serial.print(data.presion);
        Serial.println("hPa");
        
        return data;
    }
};

// ======================
// CLASE DataFilter (SIMULACION)
// ======================
class DataFilter {
private:
    vector<float> historialTemperatura;
    vector<float> historialHumedad;
    vector<float> historialPresion;
    const int MAX_HISTORIAL = 20;

public:
    void addData(float temp, float hum, float pres) {
        historialTemperatura.push_back(temp);
        historialHumedad.push_back(hum);
        historialPresion.push_back(pres);
        
        // Mantener solo los ultimos MAX_HISTORIAL valores
        if (historialTemperatura.size() > MAX_HISTORIAL) {
            historialTemperatura.erase(historialTemperatura.begin());
            historialHumedad.erase(historialHumedad.begin());
            historialPresion.erase(historialPresion.begin());
        }
    }

    FilteredData filter() {
        FilteredData result = {0, 0, 0};
        
        if (historialTemperatura.empty()) {
            return result;
        }

        float sumaTemp = 0, sumaHum = 0, sumaPres = 0;
        for (size_t i = 0; i < historialTemperatura.size(); i++) {
            sumaTemp += historialTemperatura[i];
            sumaHum += historialHumedad[i];
            sumaPres += historialPresion[i];
        }

        result.temperatura = sumaTemp / historialTemperatura.size();
        result.humedad = sumaHum / historialHumedad.size();
        result.presion = sumaPres / historialPresion.size();

        Serial.print("FILTRADO - T:");
        Serial.print(result.temperatura);
        Serial.print("C H:");
        Serial.print(result.humedad);
        Serial.print("% P:");
        Serial.print(result.presion);
        Serial.println("hPa");

        return result;
    }

    float calculateHumidityTrend() {
        if (historialHumedad.size() < 2) return 0;
        
        // Calcular tendencia real basada en datos historicos
        float sumaX = 0, sumaY = 0, sumaXY = 0, sumaX2 = 0;
        int n = historialHumedad.size();
        
        for (int i = 0; i < n; i++) {
            sumaX += i;
            sumaY += historialHumedad[i];
            sumaXY += i * historialHumedad[i];
            sumaX2 += i * i;
        }
        
        float pendiente = (n * sumaXY - sumaX * sumaY) / (n * sumaX2 - sumaX * sumaX);
        
        Serial.print("   Tendencia humedad: ");
        Serial.println(pendiente);
        return pendiente;
    }

    float calculatePressureTrend() {
        if (historialPresion.size() < 2) return 0;
        
        // Calcular tendencia de presion
        float sumaX = 0, sumaY = 0, sumaXY = 0, sumaX2 = 0;
        int n = historialPresion.size();
        
        for (int i = 0; i < n; i++) {
            sumaX += i;
            sumaY += historialPresion[i];
            sumaXY += i * historialPresion[i];
            sumaX2 += i * i;
        }
        
        float pendiente = (n * sumaXY - sumaX * sumaY) / (n * sumaX2 - sumaX * sumaX);
        
        Serial.print("   Tendencia presion: ");
        Serial.println(pendiente);
        return pendiente;
    }
};

// ======================
// CLASE PredictionEngine (SIMULACION)
// ======================
class PredictionEngine {
public:
    int predict(float temperatura, float humedad, float presion, float tendenciaHumedad, float tendenciaPresion) {
        int nivelAlerta = 0;
        int puntosRiesgo = 0;

        // ANALISIS MULTIVARIABLE PARA PREDICCION DE LLUVIA
        
        // Factor 1: Humedad alta
        if (humedad > 85) puntosRiesgo += 3;
        else if (humedad > 75) puntosRiesgo += 2;
        else if (humedad > 65) puntosRiesgo += 1;

        // Factor 2: Presion baja (señal de sistema meteorologico inestable)
        if (presion < 1005) puntosRiesgo += 3;
        else if (presion < 1010) puntosRiesgo += 2;
        else if (presion < 1015) puntosRiesgo += 1;

        // Factor 3: Tendencia de humedad creciente
        if (tendenciaHumedad > 0.4) puntosRiesgo += 2;
        else if (tendenciaHumedad > 0.2) puntosRiesgo += 1;

        // Factor 4: Tendencia de presion decreciente (muy importante)
        if (tendenciaPresion < -0.3) puntosRiesgo += 3;
        else if (tendenciaPresion < -0.1) puntosRiesgo += 2;

        // Factor 5: Temperatura estable o descendiendo
        if (temperatura < 25) puntosRiesgo += 1;  // Temperaturas bajas favorecen condensacion

        // EVALUACION FINAL DE RIESGO
        if (puntosRiesgo >= 8 || (humedad > 90 && presion < 1010)) {
            nivelAlerta = 2;
            Serial.println("PREDICCION: ALERTA ROJA - Lluvia inminente (Condiciones optimas)");
        }
        else if (puntosRiesgo >= 5) {
            nivelAlerta = 1;
            Serial.println("PREDICCION: ALERTA AMARILLA - Posible lluvia (Condiciones favorables)");
        }
        else {
            nivelAlerta = 0;
            Serial.println("PREDICCION: NORMAL - Condiciones estables");
        }

        Serial.print("   Puntos de riesgo: ");
        Serial.println(puntosRiesgo);

        return nivelAlerta;
    }
};

// ======================
// CLASE HttpClientBackend (SIMULACION)
// ======================
class HttpClientBackend {
public:
    void begin() {
        Serial.println("HttpClientBackend inicializado (simulacion)");
    }

    bool sendData(float temperatura, float humedad, float presion, int alerta) {
        Serial.println("ENVIANDO AL BACKEND:");
        
        // Simular JSON
        cout << "{" << endl;
        cout << "  \"sensor_id\": \"ARDUINO_TROPICAL_01\"," << endl;
        cout << "  \"timestamp\": " << millis() << "," << endl;
        cout << "  \"temperatura\": " << temperatura << "," << endl;
        cout << "  \"humedad\": " << humedad << "," << endl;
        cout << "  \"presion\": " << presion << "," << endl;
        cout << "  \"alerta\": " << alerta << "," << endl;
        cout << "  \"modo\": \"simulacion\"" << endl;
        cout << "}" << endl;
        
        Serial.println("(Modo simulacion - envio simulado)");
        Serial.println("Datos enviados correctamente");
        Serial.println("------------------------------------");
        
        return true;
    }
};

// ======================
// PROGRAMA PRINCIPAL
// ======================
int main() {
    srand(time(NULL));
    
    // Instancias
    SensorController sensorController;
    DataFilter dataFilter;
    PredictionEngine predictionEngine;
    HttpClientBackend httpBackend;
    
    FilteredData datosFiltrados;
    unsigned long ultimaLectura = 0;
    unsigned long ultimoFiltrado = 0;
    unsigned long ultimoEnvio = 0;

    // Funciones locales
    auto leerSensores = [&]() {
        auto datos = sensorController.readSensors();
        if (datos.temperatura > 0 && datos.humedad > 0) {
            dataFilter.addData(datos.temperatura, datos.humedad, datos.presion);
        }
    };

    auto filtrarDatos = [&]() {
        datosFiltrados = dataFilter.filter();
        if (datosFiltrados.humedad > 0) {
            float tendenciaHumedad = dataFilter.calculateHumidityTrend();
            float tendenciaPresion = dataFilter.calculatePressureTrend();
            predictionEngine.predict(datosFiltrados.temperatura, datosFiltrados.humedad, 
                                   datosFiltrados.presion, tendenciaHumedad, tendenciaPresion);
        }
    };

    auto enviarAlBackend = [&]() {
        if (datosFiltrados.humedad > 0) {
            float tendenciaHumedad = dataFilter.calculateHumidityTrend();
            float tendenciaPresion = dataFilter.calculatePressureTrend();
            int alerta = predictionEngine.predict(datosFiltrados.temperatura, datosFiltrados.humedad, 
                                                datosFiltrados.presion, tendenciaHumedad, tendenciaPresion);
            httpBackend.sendData(datosFiltrados.temperatura, datosFiltrados.humedad, 
                               datosFiltrados.presion, alerta);
        }
    };

    // SETUP
    Serial.begin(9600);
    
    Serial.println("====================================");
    Serial.println("MODO SIMULACION NATIVO ACTIVADO");
    Serial.println("====================================");

    sensorController.begin();
    httpBackend.begin();

    // LOOP
    while (true) {
        unsigned long tiempoActual = millis();
        
        // 1. LECTURA DE SENSORES (cada 5 segundos)
        if (tiempoActual - ultimaLectura >= INTERVALO_LECTURA) {
            ultimaLectura = tiempoActual;
            leerSensores();
        }
        
        // 2. FILTRADO DE DATOS (cada 30 segundos)
        if (tiempoActual - ultimoFiltrado >= INTERVALO_FILTRADO) {
            ultimoFiltrado = tiempoActual;
            filtrarDatos();
        }
        
        // 3. ENVIO AL BACKEND (cada 60 segundos)
        if (tiempoActual - ultimoEnvio >= INTERVALO_ENVIO) {
            ultimoEnvio = tiempoActual;
            enviarAlBackend();
        }
        
        delay(1000);
    }
    
    return 0;
}