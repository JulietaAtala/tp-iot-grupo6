#include "BotTelegram.h"

BotTelegram::BotTelegram(const char* token, const char* chatAutorizado, unsigned long intervaloMs)
    : bot(token, cliente), chatAutorizado(chatAutorizado),
      consulta(intervaloMs, true), cantidad(0), indice(0) {}

void BotTelegram::begin() {
    cliente.setCACert(TELEGRAM_CERTIFICATE_ROOT);   // verifica que el servidor sea Telegram
    consulta.iniciar();
}

bool BotTelegram::recibir(String& comando) {
    if (indice >= cantidad) {
        if (!consulta.cumplido()) {
            return false;
        }
        cantidad = bot.getUpdates(bot.last_message_received + 1);
        indice = 0;
    }

    while (indice < cantidad) {
        telegramMessage& mensaje = bot.messages[indice++];

        if (mensaje.type == "callback_query") {
            bot.answerCallbackQuery(mensaje.query_id);   // quita el "cargando" del boton
        }
        if (mensaje.chat_id != chatAutorizado) {
            Serial.printf("Telegram: mensaje ignorado del chat %s\r\n", mensaje.chat_id.c_str());
            continue;
        }

        comando = mensaje.text;
        comando.trim();
        comando.toLowerCase();
        return true;
    }
    return false;
}

bool BotTelegram::enviar(const String& texto) {
    return bot.sendMessage(chatAutorizado, texto, "");
}

bool BotTelegram::enviarConTeclado(const String& texto, const String& tecladoJson) {
    return bot.sendMessageWithInlineKeyboard(chatAutorizado, texto, "", tecladoJson);
}
