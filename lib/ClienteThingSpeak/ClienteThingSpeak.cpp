#include "ClienteThingSpeak.h"

namespace {
const unsigned long LIMITE_THINGSPEAK_MS = 15000;   // minimo entre escrituras (cuenta gratuita)
const int CODIGO_OK = 200;
const int CANT_CAMPOS = 8;
}

ClienteThingSpeak::ClienteThingSpeak(unsigned long canalId, const char* writeKey, unsigned long intervaloMinMs)
    : canalId(canalId), writeKey(writeKey),
      intervaloMin(intervaloMinMs < LIMITE_THINGSPEAK_MS ? LIMITE_THINGSPEAK_MS : intervaloMinMs),
      cliente(nullptr), pendiente(false), huboEnvio(false), ultimoIntento(0),
      codigo(0), resultadoNuevo(false) {
    for (int i = 0; i < CANT_CAMPOS; i++) {
        valores[i] = 0.0f;
        definido[i] = false;
    }
}

void ClienteThingSpeak::begin(WiFiClient& c) {
    cliente = &c;
    ThingSpeak.begin(c);
}

void ClienteThingSpeak::setCampo(int numero, float valor) {
    if (numero < 1 || numero > CANT_CAMPOS || isnan(valor)) {
        return;                                      // un valor invalido no se envia
    }
    valores[numero - 1] = valor;
    definido[numero - 1] = true;
}

void ClienteThingSpeak::solicitarEnvio() {
    pendiente = true;
}

bool ClienteThingSpeak::hayEnvioPendiente() const {
    return pendiente;
}

void ClienteThingSpeak::update(bool hayWifi) {
    if (!pendiente || !hayWifi) {
        return;                                      // el pedido se conserva hasta que haya Wi-Fi
    }
    if (!puedeEnviar()) {
        return;                                      // el pedido espera a que pase el limite de 15 s
    }
    enviar();
}

bool ClienteThingSpeak::huboResultado() {
    if (resultadoNuevo) {
        resultadoNuevo = false;                      // se consume: no se repite
        return true;
    }
    return false;
}

int ClienteThingSpeak::ultimoCodigo() const {
    return codigo;
}

bool ClienteThingSpeak::ultimoEnvioOk() const {
    return huboEnvio && codigo == CODIGO_OK;
}

unsigned long ClienteThingSpeak::msParaProximoEnvio() const {
    if (!huboEnvio) {
        return 0;
    }
    unsigned long transcurrido = millis() - ultimoIntento;
    return (transcurrido >= intervaloMin) ? 0 : (intervaloMin - transcurrido);
}

bool ClienteThingSpeak::puedeEnviar() const {
    return msParaProximoEnvio() == 0;
}

void ClienteThingSpeak::enviar() {
    bool hayCampos = false;
    for (int i = 0; i < CANT_CAMPOS; i++) {
        if (definido[i]) {
            ThingSpeak.setField(i + 1, valores[i]);
            hayCampos = true;
        }
    }

    if (!hayCampos) {
        pendiente = false;                           // nada para enviar
        return;
    }

    codigo = ThingSpeak.writeFields(canalId, writeKey);   // espera la respuesta del servidor
    ultimoIntento = millis();
    huboEnvio = true;
    resultadoNuevo = true;
    pendiente = false;

    if (codigo != CODIGO_OK && cliente != nullptr) {
        // Si fallo antes de enviar, la libreria puede conservar campos viejos.
        // begin() los borra, asi el proximo envio no arrastra datos desactualizados.
        ThingSpeak.begin(*cliente);
    }

    for (int i = 0; i < CANT_CAMPOS; i++) {
        definido[i] = false;                         // cada escritura lleva solo campos nuevos
    }
}

const char* ClienteThingSpeak::describirCodigo(int codigo) {
    switch (codigo) {
        case 200:  return "OK";
        case 400:  return "pedido incorrecto";
        case 404:  return "canal o clave incorrectos";
        case -101: return "valor fuera de rango";
        case -201: return "numero de campo invalido";
        case -210: return "no hay campos para enviar";
        case -301: return "no se pudo conectar a ThingSpeak";
        case -302: return "falla inesperada";
        case -303: return "respuesta no valida";
        case -304: return "tiempo de espera agotado";
        case -401: return "no insertado (limite de 15 s)";
        default:   return "error desconocido";
    }
}