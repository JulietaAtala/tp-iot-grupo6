#ifndef ACTUADOR_H
#define ACTUADOR_H

#include <Arduino.h>
#include <Temporizador.h>

/**
 * Salida digital (LED o rele) con apagado automatico no bloqueante.
 * Hay que llamar a update() en cada vuelta del loop().
 */
class Actuador {
public:
    explicit Actuador(uint8_t pin, bool activoEnAlto = true);

    void setIndicador(uint8_t pinLed);     // LED que refleja el estado (llamar ANTES de begin)
    void begin();                          // configura los pines (llamar en setup)
    void encender();
    void apagar();
    void alternar();
    void encenderPor(unsigned long ms);    // enciende y se apaga solo luego de 'ms'
    void update();                         // atiende el apagado automatico

    bool estaEncendido() const;
    const char* estadoTexto() const;       // "encendido" / "apagado" (sin tildes, apto OLED)
    unsigned long msRestantes() const;     // tiempo que falta para el apagado automatico

private:
    void escribir(bool encendido);

    uint8_t pin;
    bool activoEnAlto;
    int pinIndicador;                      // -1 si no hay LED indicador
    bool encendido;
    Temporizador temporizador;             // one-shot para el apagado automatico
};

#endif