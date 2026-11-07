#include "GestorWiFi.h"

GestorWiFi::GestorWiFi(const char* ssid, const char* clave, unsigned long reintentoMs)
    : nombreRed(ssid), clave(clave), conectado(false), cambio(false),
      reintento(reintentoMs, true) {}

void GestorWiFi::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(nombreRed, clave);
    conectado = false;
    reintento.iniciar();                           // si el primer intento falla, se reintenta
}

bool GestorWiFi::esperarConexion(unsigned long timeoutMs) {
    unsigned long inicio = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - inicio) < timeoutMs) {
        delay(100);
    }
    update();                                      // registra el estado y el cambio
    return conectado;
}

void GestorWiFi::update() {
    bool ahora = (WiFi.status() == WL_CONNECTED);

    if (ahora != conectado) {                      // cambio de estado
        conectado = ahora;
        cambio = true;
        if (conectado) {
            reintento.detener();
        } else {
            reintento.iniciar();                   // se perdio la conexion: empezar a reintentar
        }
    }

    if (!conectado && reintento.cumplido()) {
        WiFi.disconnect();
        WiFi.begin(nombreRed, clave);
    }
}

bool GestorWiFi::estaConectado() const {
    return conectado;
}

bool GestorWiFi::huboCambio() {
    if (cambio) {
        cambio = false;                            // se consume: no se repite
        return true;
    }
    return false;
}

String GestorWiFi::ip() const {
    return conectado ? WiFi.localIP().toString() : String("0.0.0.0");
}

int GestorWiFi::rssi() const {
    return conectado ? WiFi.RSSI() : 0;
}

const char* GestorWiFi::ssid() const {
    return nombreRed;
}