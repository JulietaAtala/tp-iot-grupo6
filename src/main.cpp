// Codigo fuente en C++
// TPI - Supervision ambiental y actuacion remota (ESP32 + ThingSpeak + Telegram)

// ---- librerias integradas con Arduino ----
#include <Arduino.h>

// ---- librerias propias ----
#include <Temporizador.h>
#include <Actuador.h>
#include <Potenciometro.h>

// ---- definiciones globales ----
const byte PIN_LED_VERDE = 23;   // actuador principal ("rele")
const byte PIN_LED_AZUL  = 2;    // indicador de alerta
const byte PIN_POTE      = 32;   // entrada analogica (ADC1)

const unsigned long BAUDIOS = 115200;
const unsigned long LECTURA_POTE_MS = 500;
const float UMBRAL_CRITICO_V = 2.8f;     // por encima de este voltaje: alerta

Actuador rele(PIN_LED_VERDE);
Actuador ledAzul(PIN_LED_AZUL);
Potenciometro pote(PIN_POTE, UMBRAL_CRITICO_V);
Temporizador lecturaPote(LECTURA_POTE_MS);

// ---- prototipos de funciones ----
void atenderPotenciometro();

void setup() {
    Serial.begin(BAUDIOS);
    rele.begin();
    ledAzul.begin();
    pote.begin();
    lecturaPote.iniciar();

    Serial.println("Fase 2: Potenciometro con umbral y alerta");
}

void loop() {
    rele.update();
    atenderPotenciometro();
}

// ---- cuerpo de las funciones ----

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