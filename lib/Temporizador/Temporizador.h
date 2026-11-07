#ifndef TEMPORIZADOR_H
#define TEMPORIZADOR_H

#include <Arduino.h>

/**
 * Temporizador no bloqueante basado en millis().
 * Reemplaza al delay() y al patron "if (millis() - ant >= INTERVALO)".
 *
 * Modo periodico: cumplido() devuelve true cada 'intervalo' ms y se rearma solo.
 * Modo unico (one-shot): cumplido() devuelve true una sola vez y se detiene.
 */
class Temporizador {
public:
    explicit Temporizador(unsigned long intervaloMs = 0, bool periodico = true);

    void setIntervalo(unsigned long intervaloMs);
    void iniciar();               // arranca (o reinicia) la cuenta desde ahora
    void detener();               // cancela la cuenta
    bool estaActivo() const;      // true si esta contando
    bool cumplido();              // true cuando vencio el intervalo
    unsigned long restante() const; // ms que faltan (0 si no esta activo o ya vencio)

private:
    unsigned long intervalo;
    unsigned long inicio;
    bool periodico;
    bool activo;
};

#endif