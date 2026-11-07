#ifndef SECRETS_H
#define SECRETS_H

// Credenciales y claves del proyecto.
// IMPORTANTE: este archivo NO debe subirse a un repositorio publico (agregar a .gitignore).

// ---- Wi-Fi ----
// Dejar la linea siguiente para simular en Wokwi.
// Comentarla (o borrarla) para compilar para la placa UTN.
#define USAR_WOKWI

#ifdef USAR_WOKWI
    #define WIFI_SSID "Wokwi-GUEST"
    #define WIFI_PASS ""
#else
    #define WIFI_SSID "UTNFRC"      // red de la placa UTN (confirmar con la catedra)
    #define WIFI_PASS ""
#endif

// ---- Telegram (fase 7) ----
// #define TELEGRAM_TOKEN "COMPLETAR_CON_EL_TOKEN_DE_BOTFATHER"

// ---- ThingSpeak (fase 6) ----
// #define THINGSPEAK_CHANNEL_ID 0
// #define THINGSPEAK_WRITE_KEY "COMPLETAR_CON_LA_WRITE_API_KEY"

#endif