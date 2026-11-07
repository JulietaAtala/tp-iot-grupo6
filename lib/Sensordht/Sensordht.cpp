#include "SensorDHT.h"

SensorDHT::SensorDHT(uint8_t pin, uint8_t tipo)
    : sensor(pin, tipo), temp(NAN), hum(NAN),
      hayDatos(false), lecturaOk(false), fallos(0) {}

void SensorDHT::begin() {
    sensor.begin();
}

bool SensorDHT::update() {
    float t = sensor.readTemperature();
    float h = sensor.readHumidity();

    if (isnan(t) || isnan(h)) {
        lecturaOk = false;
        fallos++;                // se conservan los ultimos valores buenos
        return false;
    }

    temp = t;
    hum = h;
    hayDatos = true;
    lecturaOk = true;
    fallos = 0;
    return true;
}

bool SensorDHT::tieneDatos() const {
    return hayDatos;
}

bool SensorDHT::ultimaLecturaOk() const {
    return lecturaOk;
}

float SensorDHT::temperatura() const {
    return temp;
}

float SensorDHT::humedad() const {
    return hum;
}

unsigned int SensorDHT::fallosConsecutivos() const {
    return fallos;
}