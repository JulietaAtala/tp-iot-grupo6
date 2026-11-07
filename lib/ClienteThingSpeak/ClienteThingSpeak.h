#ifndef CLIENTE_THINGSPEAK_H
#define CLIENTE_THINGSPEAK_H

#include <Arduino.h>
#include <WiFi.h>
#include <ThingSpeak.h>

/**
 * Envio de datos a un canal de ThingSpeak respetando su limite de escritura.
 *
 * ThingSpeak (cuenta gratuita) solo acepta UNA escritura cada 15 s. Por eso:
 *  - setCampo() solo guarda el valor; no envia nada.
 *  - solicitarEnvio() marca que hay datos para enviar.
 *  - update() (en cada vuelta del loop) envia cuando hay Wi-Fi y ya paso el tiempo minimo.
 *    Si dos pedidos coinciden (envio periodico + /thingup), viajan juntos en UNA sola escritura.
 *
 * Cada escritura lleva solo los campos cargados desde la escritura anterior.
 * Nota: la escritura HTTP (writeFields) es la unica operacion que espera la respuesta del
 * servidor, normalmente menos de 1 s.
 */
class ClienteThingSpeak {
public:
    ClienteThingSpeak(unsigned long canalId, const char* writeKey, unsigned long intervaloMinMs = 20000);

    void begin(WiFiClient& cliente);                 // llamar en setup, con un WiFiClient global

    void setCampo(int numero, float valor);          // campo 1 a 8; ignora NaN y numeros invalidos
    void solicitarEnvio();                           // pide enviar los campos cargados
    bool hayEnvioPendiente() const;

    void update(bool hayWifi);                       // envia si corresponde

    bool huboResultado();                            // true UNA vez despues de cada intento de envio
    int  ultimoCodigo() const;                       // 200 = OK; otro valor = error
    bool ultimoEnvioOk() const;
    unsigned long msParaProximoEnvio() const;        // 0 si se puede enviar ya

    static const char* describirCodigo(int codigo);  // texto en espanol para mostrar al usuario

private:
    bool puedeEnviar() const;
    void enviar();

    unsigned long canalId;
    const char* writeKey;
    unsigned long intervaloMin;
    WiFiClient* cliente;

    float valores[8];
    bool definido[8];
    bool pendiente;

    bool huboEnvio;                                  // hubo al menos un intento
    unsigned long ultimoIntento;
    int codigo;
    bool resultadoNuevo;
};

#endif