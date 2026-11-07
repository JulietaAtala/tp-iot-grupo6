#ifndef GESTOR_WIFI_H
#define GESTOR_WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <Temporizador.h>

/**
 * Conexion Wi-Fi (modo estacion) con reconexion automatica NO bloqueante.
 *
 *  - begin() inicia la conexion y vuelve enseguida.
 *  - esperarConexion() es la UNICA funcion bloqueante: se usa en setup(), con un tiempo maximo,
 *    para arrancar ya conectado. Si vence el tiempo, el programa sigue y update() reintenta.
 *  - update() se llama en cada vuelta del loop(): detecta la perdida de conexion y reintenta
 *    cada 'reintentoMs'.
 *  - huboCambio() permite avisar (Serial, OLED) solo cuando la conexion se establece o se pierde.
 */
class GestorWiFi {
public:
    GestorWiFi(const char* ssid, const char* clave, unsigned long reintentoMs = 10000);

    void begin();                                  // inicia la conexion (no bloquea)
    bool esperarConexion(unsigned long timeoutMs); // bloquea hasta conectar o vencer el tiempo
    void update();                                 // detecta cambios y reintenta si hace falta

    bool estaConectado() const;
    bool huboCambio();                             // true UNA vez cada vez que cambia el estado
    String ip() const;                             // "0.0.0.0" si no hay conexion
    int rssi() const;                              // intensidad de senal en dBm
    const char* ssid() const;

private:
    const char* nombreRed;
    const char* clave;
    bool conectado;
    bool cambio;
    Temporizador reintento;                        // periodico: solo corre mientras no hay conexion
};

#endif