#include <WiFi.h>
#include <HTTPClient.h>
#include <DHTesp.h>

// Configuración Wi-Fi
const char* ssid = "WLAN_VICH";
const char* password = "********";
//const char* serverName = "http://127.0.0.1:8000/api/sensor-readings";
const char* serverName = "http://190.160.0.180:8000/api/sensor-readings";

// Pines de sensores
#define DHTPIN_INTERNAL 17 // Sensor interno (DHT22)
#define DHTPIN_EXTERNAL 16 // Sensor externo (DHT11)
#define DOPIN 4            // Sensor de lluvia

// Objetos DHT
DHTesp dhtInternal; // Sensor interno (DHT22)
DHTesp dhtExternal; // Sensor externo (DHT11)

void setup() {
    Serial.begin(115200);

    // Configuración de sensores
    dhtInternal.setup(DHTPIN_INTERNAL, DHTesp::DHT22);
    dhtExternal.setup(DHTPIN_EXTERNAL, DHTesp::DHT11);
    pinMode(DOPIN, INPUT);

    // Conexión Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConexión exitosa a Wi-Fi");
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        // Comenzar conexión con el servidor
        http.begin(serverName);
        http.addHeader("Content-Type", "application/json");

        // Obtener lecturas de los sensores
        float tempInternal = dhtInternal.getTemperature();
        float humInternal = dhtInternal.getHumidity();
        float tempExternal = dhtExternal.getTemperature();
        float humExternal = dhtExternal.getHumidity();
        String rainStatus = (digitalRead(DOPIN) == HIGH) ? "Sin lluvia detectada" : "Lluvia detectada";

        // Formar el JSON para la API
        String postData = String("{") +
            "\"temperature_internal\": " + String(tempInternal) + "," +
            "\"humidity_internal\": " + String(humInternal) + "," +
            "\"temperature_external\": " + String(tempExternal) + "," +
            "\"humidity_external\": " + String(humExternal) + "," +
            "\"rain_status\": \"" + rainStatus + "\"" +
            "}";

        // Enviar datos a la API
        int httpResponseCode = http.POST(postData);

        // Verificar la respuesta del servidor
        if (httpResponseCode > 0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);

            // Leer la respuesta del servidor (opcional)
            String response = http.getString();
            Serial.println("Response from server:");
            Serial.println(response);

            if (httpResponseCode == 200) {
                Serial.println("Datos enviados correctamente.");
            } else {
                Serial.println("Hubo un error en el servidor.");
            }
        } else {
            // Manejo de errores HTTP
            Serial.print("Error en la solicitud HTTP: ");
            Serial.println(http.errorToString(httpResponseCode).c_str());
        }

        http.end(); // Finalizar la conexión HTTP
    } else {
        // Manejo de pérdida de conexión Wi-Fi
        Serial.println("No conectado a Wi-Fi, intentando reconectar...");
        WiFi.begin(ssid, password);
    }

    delay(5000); // Esperar 5 segundos antes del siguiente ciclo
}
