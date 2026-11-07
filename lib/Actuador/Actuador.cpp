#include "Actuador.h"

Actuador::Actuador(uint8_t pin, bool activoEnAlto)
    : pin(pin), activoEnAlto(activoEnAlto), encendido(false),
      temporizador(0, false) {}

void Actuador::begin() {
    pinMode(pin, OUTPUT);
    escribir(false);                       // arranca siempre apagado
}

void Actuador::encender() {
    temporizador.detener();                // cancela un apagado automatico pendiente
    escribir(true);
}

void Actuador::apagar() {
    temporizador.detener();
    escribir(false);
}

void Actuador::alternar() {
    if (encendido) {
        apagar();
    } else {
        encender();
    }
}

void Actuador::encenderPor(unsigned long ms) {
    escribir(true);
    temporizador.setIntervalo(ms);
    temporizador.iniciar();                // si ya estaba contando, reinicia la cuenta
}

void Actuador::update() {
    if (temporizador.cumplido()) {
        escribir(false);
    }
}

bool Actuador::estaEncendido() const {
    return encendido;
}

const char* Actuador::estadoTexto() const {
    return encendido ? "encendido" : "apagado";
}

unsigned long Actuador::msRestantes() const {
    return temporizador.restante();
}

void Actuador::escribir(bool valor) {
    encendido = valor;
    digitalWrite(pin, (valor == activoEnAlto) ? HIGH : LOW);
}