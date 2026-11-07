#include "Potenciometro.h"

Potenciometro::Potenciometro(uint8_t pin, float umbralV, float histeresisV, float vref)
    : pin(pin), umbralV(umbralV), histeresisV(histeresisV), vref(vref),
      lecturaRaw(0), lecturaV(0.0f), alerta(false), alertaPendiente(false) {}

void Potenciometro::begin() {
    pinMode(pin, INPUT);
    analogReadResolution(RESOLUCION_BITS);   // 12 bits: 0 a 4095
    update();                                // primera lectura real: voltaje() no devuelve 0 por defecto
}

void Potenciometro::update() {
    lecturaRaw = leerPromediado();
    lecturaV = (lecturaRaw * vref) / MAX_ADC;

    if (!alerta && lecturaV >= umbralV) {
        alerta = true;
        alertaPendiente = true;              // cruce hacia arriba: avisar una vez
    } else if (alerta && lecturaV < (umbralV - histeresisV)) {
        alerta = false;                      // volvio a la zona segura: se rearma
    }
}

int Potenciometro::raw() const {
    return lecturaRaw;
}

float Potenciometro::voltaje() const {
    return lecturaV;
}

float Potenciometro::umbral() const {
    return umbralV;
}

void Potenciometro::setUmbral(float umbralV) {
    this->umbralV = umbralV;
}

bool Potenciometro::superaUmbral() const {
    return lecturaV >= umbralV;
}

bool Potenciometro::enAlerta() const {
    return alerta;
}

bool Potenciometro::alertaNueva() {
    if (alertaPendiente) {
        alertaPendiente = false;             // se consume: no se repite
        return true;
    }
    return false;
}

int Potenciometro::leerPromediado() const {
    long suma = 0;
    for (int i = 0; i < MUESTRAS; i++) {
        suma += analogRead(pin);
    }
    return (int)(suma / MUESTRAS);
}