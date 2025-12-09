#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <vector>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <curl/curl.h>
#include <json/json.h>

using namespace std;

// ======================
// CONFIGURACION
// ======================
const unsigned long INTERVALO_LECTURA = 5000;
const unsigned long INTERVALO_FILTRADO = 30000;
const unsigned long INTERVALO_ENVIO = 60000;

// Configuración de la API
const string API_URL = "http://localhost:4000/api/sensores";
const string API_KEY = "tu-api-key-aqui";

// ======================
// FUNCIONES UTILITARIAS MEJORADAS
// ======================
// Función para obtener timestamp UNIX en MILISEGUNDOS
unsigned long long getUnixTimestampMillis() {
    auto now = chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return chrono::duration_cast<chrono::milliseconds>(duration).count();
}

// Función para redondear a 2 decimales
float roundToTwoDecimals(float value) {
    return roundf(value * 100.0f) / 100.0f;
}

// Función para formatear float con precisión (para display)
string formatFloat(float value, int precision = 2) {
    stringstream ss;
    ss << fixed << setprecision(precision) << value;
    return ss.str();
}

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
    void println(float value) { 
        cout << fixed << setprecision(2) << value << endl;
        cout.unsetf(ios::fixed);
    }
    void print(float value) { 
        cout << fixed << setprecision(2) << value;
        cout.unsetf(ios::fixed);
    }
    void println(int value) { cout << value << endl; }
    void print(int value) { cout << value; }
    // Nuevo método para imprimir float con precisión personalizada
    void print(float value, int precision) { 
        cout << fixed << setprecision(precision) << value;
        cout.unsetf(ios::fixed);
    }
    void println(float value, int precision) { 
        cout << fixed << setprecision(precision) << value << endl;
        cout.unsetf(ios::fixed);
    }
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
// FUNCIÓN DE CALLBACK PARA CURL
// ======================
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* response) {
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

// ======================
// CLASE HttpClientBackend CORREGIDA
// ======================
class HttpClientBackend {
private:
    CURL* curl;
    string responseBuffer;
    
public:
    void begin() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (curl) {
            Serial.println("HttpClientBackend inicializado (conexion real)");
        } else {
            Serial.println("ERROR: No se pudo inicializar CURL");
        }
    }
    
    ~HttpClientBackend() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
    }

    bool sendData(float temperatura, float humedad, float presion, int alerta) {
        if (!curl) {
            Serial.println("ERROR: CURL no inicializado");
            return false;
        }
        
        // REDONDEAR VALORES A 2 DECIMALES
        float temp_rounded = roundToTwoDecimals(temperatura);
        float hum_rounded = roundToTwoDecimals(humedad);
        float pres_rounded = roundToTwoDecimals(presion);
        
        // Crear JSON con valores redondeados y timestamp en MILISEGUNDOS
        Json::Value jsonData;
        jsonData["sensor_id"] = "ARDUINO_TROPICAL_01";
        jsonData["timestamp"] = static_cast<Json::Int64>(getUnixTimestampMillis()); // MILISEGUNDOS
        jsonData["temperatura"] = temp_rounded;
        jsonData["humedad"] = hum_rounded;
        jsonData["presion"] = pres_rounded;
        jsonData["alerta"] = alerta;
        jsonData["modo"] = "simulacion_nativo";
        
        // Convertir JSON a string con indentación
        Json::StreamWriterBuilder writer;
        writer["indentation"] = "  ";
        string jsonString = Json::writeString(writer, jsonData);
        
        // Mostrar información en consola - CORREGIDO
        Serial.println("ENVIANDO A API REAL:");
        Serial.println("URL: " + API_URL);
        Serial.println("TIMESTAMP (ms): " + to_string(getUnixTimestampMillis()));
        
        // Mostrar datos redondeados usando el nuevo método
        Serial.print("DATOS REDONDEADOS: T=");
        Serial.print(temp_rounded, 2);  // Ahora funciona con el nuevo método
        Serial.print("°C, H=");
        Serial.print(hum_rounded, 2);   // Ahora funciona con el nuevo método
        Serial.print("%, P=");
        Serial.print(pres_rounded, 2);  // Ahora funciona con el nuevo método
        Serial.println(" hPa");
        
        Serial.println("JSON: " + jsonString);
        
        // Configurar la solicitud HTTP
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("X-API-Key: " + API_KEY).c_str());
        
        curl_easy_setopt(curl, CURLOPT_URL, API_URL.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonString.length());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        
        responseBuffer.clear();
        
        // Realizar la solicitud
        CURLcode res = curl_easy_perform(curl);
        
        curl_slist_free_all(headers);
        
        if (res != CURLE_OK) {
            Serial.println("ERROR en envio HTTP: " + string(curl_easy_strerror(res)));
            return false;
        }
        
        // Obtener código de respuesta HTTP
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        
        Serial.println("Respuesta HTTP: " + to_string(http_code));
        
        // Formatear respuesta JSON
        if (!responseBuffer.empty()) {
            try {
                Json::Value jsonResponse;
                Json::CharReaderBuilder readerBuilder;
                string errors;
                stringstream responseStream(responseBuffer);
                
                if (Json::parseFromStream(readerBuilder, responseStream, &jsonResponse, &errors)) {
                    Json::StreamWriterBuilder responseWriter;
                    responseWriter["indentation"] = "  ";
                    string formattedResponse = Json::writeString(responseWriter, jsonResponse);
                    Serial.println("Body respuesta (formateado):");
                    Serial.println(formattedResponse);
                } else {
                    Serial.println("Body respuesta: " + responseBuffer);
                }
            } catch (...) {
                Serial.println("Body respuesta: " + responseBuffer);
            }
        }
        
        if (http_code >= 200 && http_code < 300) {
            Serial.println("Datos enviados correctamente al backend");
            return true;
        } else {
            Serial.println("Error en respuesta del servidor");
            return false;
        }
    }
};

// ======================
// CLASE SensorController
// ======================
class SensorController {
public:
    void begin() {
        Serial.println("SensorController inicializado (simulacion)");
    }

    SensorData readSensors() {
        SensorData data;
        data.timestamp = millis();
        
        float baseHumedad = 40 + (rand() % 6000) / 100.0;
        
        if (baseHumedad > 80) {
            data.presion = 1000.0 + (rand() % 1500) / 100.0;
            data.temperatura = 18.0 + (rand() % 1500) / 100.0;
        } else if (baseHumedad > 60) {
            data.presion = 1010.0 + (rand() % 1500) / 100.0;
            data.temperatura = 22.0 + (rand() % 1800) / 100.0;
        } else {
            data.presion = 1015.0 + (rand() % 1500) / 100.0;
            data.temperatura = 25.0 + (rand() % 2000) / 100.0;
        }
        
        data.humedad = max(30.0f, min(100.0f, baseHumedad));
        
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
// CLASE DataFilter
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
// CLASE PredictionEngine
// ======================
class PredictionEngine {
public:
    int predict(float temperatura, float humedad, float presion, float tendenciaHumedad, float tendenciaPresion) {
        int nivelAlerta = 0;
        int puntosRiesgo = 0;

        if (humedad > 85) puntosRiesgo += 3;
        else if (humedad > 75) puntosRiesgo += 2;
        else if (humedad > 65) puntosRiesgo += 1;

        if (presion < 1005) puntosRiesgo += 3;
        else if (presion < 1010) puntosRiesgo += 2;
        else if (presion < 1015) puntosRiesgo += 1;

        if (tendenciaHumedad > 0.4) puntosRiesgo += 2;
        else if (tendenciaHumedad > 0.2) puntosRiesgo += 1;

        if (tendenciaPresion < -0.3) puntosRiesgo += 3;
        else if (tendenciaPresion < -0.1) puntosRiesgo += 2;

        if (temperatura < 25) puntosRiesgo += 1;

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

        Serial.print("   Puntos de riesgo: ");
        Serial.println(puntosRiesgo);

        return nivelAlerta;
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
            
            // Redondear los datos ANTES de enviar
            float temp_redondeada = roundToTwoDecimals(datosFiltrados.temperatura);
            float hum_redondeada = roundToTwoDecimals(datosFiltrados.humedad);
            float pres_redondeada = roundToTwoDecimals(datosFiltrados.presion);
            
            // Ahora envía a la API real
            bool exito = httpBackend.sendData(temp_redondeada, hum_redondeada, pres_redondeada, alerta);
            
            if (exito) {
                Serial.println("Envio exitoso a la API");
            } else {
                Serial.println(" Fallo en el envio a la API");
            }
            
            Serial.println("------------------------------------");
        }
    };

    // SETUP
    Serial.begin(9600);
    
    Serial.println("====================================");
    Serial.println("MODO SIMULACION CON API REAL");
    Serial.println("====================================");
    Serial.println("API destino: " + API_URL);
    Serial.println("Precisión: 2 decimales");
    Serial.println("Timestamp: UNIX en milisegundos");
    Serial.println("====================================");

    sensorController.begin();
    httpBackend.begin();

    // LOOP
    while (true) {
        unsigned long tiempoActual = millis();
        
        if (tiempoActual - ultimaLectura >= INTERVALO_LECTURA) {
            ultimaLectura = tiempoActual;
            leerSensores();
        }
        
        if (tiempoActual - ultimoFiltrado >= INTERVALO_FILTRADO) {
            ultimoFiltrado = tiempoActual;
            filtrarDatos();
        }
        
        if (tiempoActual - ultimoEnvio >= INTERVALO_ENVIO) {
            ultimoEnvio = tiempoActual;
            enviarAlBackend();
        }
        
        delay(1000);
    }
    
    return 0;
}