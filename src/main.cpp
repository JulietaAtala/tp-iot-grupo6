// Codigo fuente en C++
// TPI - Supervision ambiental y actuacion remota (ESP32 + ThingSpeak + Telegram)

// ---- librerias integradas con Arduino ----
#include <Arduino.h>

// ---- librerias propias ----
#include <Temporizador.h>
#include <Actuador.h>
#include <Potenciometro.h>
#include <SensorDHT.h>
#include <PantallaOLED.h>
#include <GestorWiFi.h>

// ---- credenciales (Wi-Fi, Telegram, ThingSpeak) ----
#include "secrets.h"

// ---- definiciones globales ----
const byte PIN_RELE      = 27;   // modulo rele (actuador principal)
const byte PIN_LED_VERDE = 23;   // indica que el rele esta activado
const byte PIN_LED_AZUL  = 2;    // indicador de alerta
const byte PIN_POTE      = 32;   // entrada analogica (ADC1)
const byte PIN_DHT       = 33;   // sensor DHT22
// OLED: SDA = GPIO 21, SCL = GPIO 22 (pines I2C por defecto del ESP32)

const unsigned long BAUDIOS = 115200;
const unsigned long LECTURA_POTE_MS = 500;
const unsigned long LECTURA_DHT_MS = 2000;   // el DHT22 admite ~1 lectura cada 2 s
const unsigned long ESPERA_WIFI_MS = 15000;  // tiempo maximo de espera de Wi-Fi al arrancar
const float UMBRAL_CRITICO_V = 2.8f;         // por encima de este voltaje: alerta

Actuador rele(PIN_RELE);         // el LED verde acompana al rele (ver setup)
Actuador ledAzul(PIN_LED_AZUL);
Potenciometro pote(PIN_POTE, UMBRAL_CRITICO_V);
SensorDHT dht(PIN_DHT);
PantallaOLED pantalla;
GestorWiFi wifi(WIFI_SSID, WIFI_PASS);
Temporizador lecturaPote(LECTURA_POTE_MS);
Temporizador lecturaDht(LECTURA_DHT_MS);

// ---- prototipos de funciones ----
void atenderPotenciometro();
void atenderDHT();
void informarEstadoWifi();

void setup() {
    Serial.begin(BAUDIOS);
    rele.setIndicador(PIN_LED_VERDE);
    rele.begin();
    ledAzul.begin();
    pote.begin();
    dht.begin();

    if (!pantalla.begin()) {
        Serial.println("ERROR: no se encontro la pantalla OLED (0x3C)");
    }

    Serial.println("Fase 5: GestorWiFi");
    pantalla.mostrarConTitulo("WiFi", String("Conectando a\n") + wifi.ssid() + "...");

    wifi.begin();
    wifi.esperarConexion(ESPERA_WIFI_MS);   // unica espera bloqueante del programa, y con limite
    wifi.huboCambio();                      // se descarta el aviso: el estado inicial se informa aca
    informarEstadoWifi();

    lecturaPote.iniciar();
    lecturaDht.iniciar();
}

void loop() {
    rele.update();
    pantalla.update();

    wifi.update();
    if (wifi.huboCambio()) {
        informarEstadoWifi();
    }

    atenderPotenciometro();
    atenderDHT();
}

// ---- cuerpo de las funciones ----

// Informa por Serial y en el OLED si el Wi-Fi esta conectado o no.
void informarEstadoWifi() {
    if (wifi.estaConectado()) {
        Serial.printf("WiFi conectado a %s | IP: %s | RSSI: %d dBm\r\n",
                      wifi.ssid(), wifi.ip().c_str(), wifi.rssi());
        pantalla.mostrarConTitulo("WiFi conectado",
            String("Red: ") + wifi.ssid() + "\nIP: " + wifi.ip(), 5000);
    } else {
        Serial.println("WiFi SIN conexion. Reintentando...");
        pantalla.mostrarConTitulo("WiFi", "Sin conexion\nReintentando...", 5000);
    }
}

// Lee el pote cada LECTURA_POTE_MS, informa y maneja la alerta (azul = alerta).
void atenderPotenciometro() {
    if (!lecturaPote.cumplido()) {
        return;
    }

    pote.update();
    Serial.printf("Pote: raw=%d  V=%.2f\r\n", pote.raw(), pote.voltaje());

    if (pote.alertaNueva()) {
        // En la fase 8 aca se enviara el mensaje al chat de Telegram.
        Serial.printf("ALERTA: %.2f V supera el umbral de %.2f V\r\n",
                      pote.voltaje(), pote.umbral());
    }

    // El LED refleja el estado real; el aviso (arriba) usa histeresis para no repetirse.
    if (pote.superaUmbral()) {
        ledAzul.encender();
    } else {
        ledAzul.apagar();
    }
}

// Lee el DHT22 cada LECTURA_DHT_MS. Si falla, conserva el ultimo valor valido.
void atenderDHT() {
    if (!lecturaDht.cumplido()) {
        return;
    }

    if (dht.update()) {
        Serial.printf("DHT: T=%.1f C  H=%.1f %%\r\n", dht.temperatura(), dht.humedad());
    } else if (dht.tieneDatos()) {
        Serial.printf("DHT: lectura fallida (%u). Ultimo valido: T=%.1f C  H=%.1f %%\r\n",
                      dht.fallosConsecutivos(), dht.temperatura(), dht.humedad());
    } else {
        Serial.printf("DHT: lectura fallida (%u). Aun sin datos validos\r\n",
                      dht.fallosConsecutivos());
    }
}