#ifndef SENSOR_DHT_H
#define SENSOR_DHT_H

#include <Arduino.h>
#include <DHT.h>

/**
 * Sensor de temperatura y humedad DHT22.
 *  - update() lee el sensor y guarda la ultima lectura VALIDA.
 *  - Si una lectura falla (NaN), se conserva el ultimo valor bueno y se cuentan los fallos.
 *  - temperatura()/humedad() no leen el sensor: devuelven los ultimos valores registrados
 *    (es lo que necesita el comando /clima).
 *
 * El DHT22 admite como maximo una lectura cada ~2 s: llamar a update() con un Temporizador.
 */
class SensorDHT {
public:
    explicit SensorDHT(uint8_t pin, uint8_t tipo = DHT22);

    void begin();                             // inicializa el sensor (llamar en setup)
    bool update();                            // lee el sensor; true si la lectura fue valida

    bool  tieneDatos() const;                 // true si hubo al menos una lectura valida
    bool  ultimaLecturaOk() const;            // true si la ULTIMA lectura intentada fue valida
    float temperatura() const;                // ultima temperatura valida en °C (NAN si no hay datos)
    float humedad() const;                    // ultima humedad relativa valida en % (NAN si no hay datos)
    unsigned int fallosConsecutivos() const;  // lecturas fallidas seguidas (0 si la ultima fue buena)

private:
    DHT sensor;
    float temp;
    float hum;
    bool hayDatos;
    bool lecturaOk;
    unsigned int fallos;
};

#endif