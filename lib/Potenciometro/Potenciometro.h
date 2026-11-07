#ifndef POTENCIOMETRO_H
#define POTENCIOMETRO_H

#include <Arduino.h>

/**
 * Potenciometro conectado a una entrada analogica (ADC) del ESP32.
 *  - Promedia varias muestras para reducir el ruido del ADC.
 *  - Convierte la lectura a voltaje (0 a VREF).
 *  - Detecta cuando se supera un umbral critico, con histeresis, para
 *    generar UNA sola alerta por cada vez que se cruza el umbral.
 *
 * Usar update() periodicamente (por ejemplo con un Temporizador).
 */
class Potenciometro {
public:
    Potenciometro(uint8_t pin, float umbralV = 2.8f, float histeresisV = 0.2f, float vref = 3.3f);

    void begin();                    // configura el pin y la resolucion del ADC (llamar en setup)
    void update();                   // lee, calcula el voltaje y evalua el umbral

    int   raw() const;               // ultima lectura (0 a 4095)
    float voltaje() const;           // ultimo voltaje calculado (0 a VREF)
    float umbral() const;
    void  setUmbral(float umbralV);

    bool superaUmbral() const;       // estado instantaneo: true si el voltaje actual >= umbral (para LED/display)
    bool enAlerta() const;           // alerta "latcheada": se mantiene hasta bajar de (umbral - histeresis)
    bool alertaNueva();              // true UNA sola vez cuando se cruza el umbral hacia arriba (para el aviso)

private:
    int leerPromediado() const;

    uint8_t pin;
    float umbralV;
    float histeresisV;
    float vref;

    int lecturaRaw;
    float lecturaV;
    bool alerta;                     // estado actual (por encima del umbral)
    bool alertaPendiente;            // aviso aun no consumido por alertaNueva()

    static const int RESOLUCION_BITS = 12;
    static const int MAX_ADC = (1 << RESOLUCION_BITS) - 1;   // 4095
    static const int MUESTRAS = 8;
};

#endif