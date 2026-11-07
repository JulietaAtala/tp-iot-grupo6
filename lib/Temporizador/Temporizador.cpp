#include "Temporizador.h"

Temporizador::Temporizador(unsigned long intervaloMs, bool periodico)
    : intervalo(intervaloMs), inicio(0), periodico(periodico), activo(false) {}

void Temporizador::setIntervalo(unsigned long intervaloMs) {
    intervalo = intervaloMs;
}

void Temporizador::iniciar() {
    inicio = millis();
    activo = true;
}

void Temporizador::detener() {
    activo = false;
}

bool Temporizador::estaActivo() const {
    return activo;
}

bool Temporizador::cumplido() {
    if (!activo) {
        return false;
    }
    // La resta sin signo funciona aunque millis() desborde (~49 dias).
    if (millis() - inicio >= intervalo) {
        if (periodico) {
            inicio = millis();   // rearmar para el proximo ciclo
        } else {
            activo = false;      // one-shot: se usa una sola vez
        }
        return true;
    }
    return false;
}

unsigned long Temporizador::restante() const {
    if (!activo) {
        return 0;
    }
    unsigned long transcurrido = millis() - inicio;
    return (transcurrido >= intervalo) ? 0 : (intervalo - transcurrido);
}