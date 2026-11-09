#ifndef BOT_TELEGRAM_H
#define BOT_TELEGRAM_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Temporizador.h>

/**
 * Bot de Telegram que solo atiende a un chat autorizado.
 * Consulta mensajes nuevos cada 'intervaloMs' (no bloquea entre consultas).
 * Los botones del teclado inline llegan como texto, igual que un comando escrito.
 */
class BotTelegram {
public:
    BotTelegram(const char* token, const char* chatAutorizado, unsigned long intervaloMs = 1000);

    void begin();                                   // llamar en setup
    bool recibir(String& comando);                  // true si llego un comando (en minusculas)
    bool enviar(const String& texto);               // envia al chat autorizado
    bool enviarConTeclado(const String& texto, const String& tecladoJson);

private:
    WiFiClientSecure cliente;
    UniversalTelegramBot bot;
    const char* chatAutorizado;
    Temporizador consulta;
    int cantidad;                                   // mensajes recibidos en la ultima consulta
    int indice;                                     // proximo mensaje a entregar
};

#endif
