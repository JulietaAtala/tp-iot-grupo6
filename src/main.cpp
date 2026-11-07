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
#include <ClienteThingSpeak.h>

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
const unsigned long ENVIO_THINGSPEAK_MS = 20000;   // ThingSpeak gratuito: minimo 15 s entre envios
const float UMBRAL_CRITICO_V = 2.8f;         // por encima de este voltaje: alerta

// Campos del canal de ThingSpeak
const int CAMPO_TEMPERATURA = 1;
const int CAMPO_HUMEDAD     = 2;
const int CAMPO_VOLTAJE     = 3;             // lo usara /thingup (fase 8)

WiFiClient clienteWifi;          // conexion que usa ThingSpeak

Actuador rele(PIN_RELE);         // el LED verde acompana al rele (ver setup)
Actuador ledAzul(PIN_LED_AZUL);
Potenciometro pote(PIN_POTE, UMBRAL_CRITICO_V);
SensorDHT dht(PIN_DHT);
PantallaOLED pantalla;
GestorWiFi wifi(WIFI_SSID, WIFI_PASS);
ClienteThingSpeak thingspeak(THINGSPEAK_CHANNEL_ID, THINGSPEAK_WRITE_KEY, ENVIO_THINGSPEAK_MS);
Temporizador lecturaPote(LECTURA_POTE_MS);
Temporizador lecturaDht(LECTURA_DHT_MS);
Temporizador envioThingSpeak(ENVIO_THINGSPEAK_MS);

// ---- prototipos de funciones ----
void atenderPotenciometro();
void atenderDHT();
void atenderThingSpeak();
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

    Serial.println("Fase 6: ClienteThingSpeak");
    thingspeak.begin(clienteWifi);
    pantalla.mostrarConTitulo("WiFi", String("Conectando a\n") + wifi.ssid() + "...");

    wifi.begin();
    wifi.esperarConexion(ESPERA_WIFI_MS);   // unica espera bloqueante del programa, y con limite
    wifi.huboCambio();                      // se descarta el aviso: el estado inicial se informa aca
    informarEstadoWifi();

    lecturaPote.iniciar();
    lecturaDht.iniciar();
    envioThingSpeak.iniciar();
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
    atenderThingSpeak();
}

// ---- cuerpo de las funciones ----

// Cada ENVIO_THINGSPEAK_MS carga temperatura y humedad y pide enviarlas a ThingSpeak.
// ClienteThingSpeak decide el momento exacto (Wi-Fi disponible y limite de 15 s).
void atenderThingSpeak() {
    if (envioThingSpeak.cumplido() && dht.tieneDatos()) {
        thingspeak.setCampo(CAMPO_TEMPERATURA, dht.temperatura());
        thingspeak.setCampo(CAMPO_HUMEDAD, dht.humedad());
        thingspeak.solicitarEnvio();
    }

    thingspeak.update(wifi.estaConectado());

    if (thingspeak.huboResultado()) {
        int codigo = thingspeak.ultimoCodigo();
        Serial.printf("ThingSpeak: %s (codigo %d)\r\n",
                      ClienteThingSpeak::describirCodigo(codigo), codigo);
    }
}

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