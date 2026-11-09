// Codigo fuente en C++
// TPI - Supervision ambiental y actuacion remota (ESP32 + ThingSpeak + Telegram)

// ---- librerias integradas con Arduino ----
#include <Arduino.h>

// ---- librerias propias ----
#include <Temporizador.h>
#include <Actuador.h>
#include <Potenciometro.h>
#include <SensorDHT.h>
#include <PantallaOLED.h>
#include <GestorWiFi.h>
#include <ClienteThingSpeak.h>
#include <BotTelegram.h>

// ---- credenciales (Wi-Fi, Telegram, ThingSpeak) ----
#include "secrets.h"

// ---- definiciones globales ----
const byte PIN_RELE      = 27;   // modulo rele (actuador principal)
const byte PIN_LED_VERDE = 23;   // indica que el rele esta activado
const byte PIN_LED_AZUL  = 2;    // indicador de alerta
const byte PIN_POTE      = 32;   // entrada analogica (ADC1)
const byte PIN_DHT       = 33;   // sensor DHT22
// OLED: SDA = GPIO 21, SCL = GPIO 22 (pines I2C por defecto del ESP32)

const unsigned long BAUDIOS = 115200;
const unsigned long LECTURA_POTE_MS = 500;
const unsigned long LECTURA_DHT_MS = 2000;         // el DHT22 admite ~1 lectura cada 2 s
const unsigned long ESPERA_WIFI_MS = 15000;        // tiempo maximo de espera de Wi-Fi al arrancar
const unsigned long ENVIO_THINGSPEAK_MS = 20000;   // ThingSpeak gratuito: minimo 15 s entre envios
const unsigned long CONSULTA_TELEGRAM_MS = 1000;
const float UMBRAL_CRITICO_V = 2.8f;               // por encima de este voltaje: alerta
const long RELE_MAX_SEGUNDOS = 3600;

// Campos del canal de ThingSpeak
const int CAMPO_TEMPERATURA = 1;
const int CAMPO_HUMEDAD     = 2;
const int CAMPO_VOLTAJE     = 3;

// Teclado inline de /start: cada boton envia su comando como si se lo escribiera
const char* TECLADO_INICIO =
    "[[{\"text\":\"Ver clima\",\"callback_data\":\"/clima\"}],"
    "[{\"text\":\"Encender rele 10 s\",\"callback_data\":\"/releon 10\"},"
    "{\"text\":\"Apagar rele\",\"callback_data\":\"/releoff\"}],"
    "[{\"text\":\"Enviar voltaje\",\"callback_data\":\"/thingup\"}],"
    "[{\"text\":\"Display LED\",\"callback_data\":\"/displayled\"},"
    "{\"text\":\"Display pote\",\"callback_data\":\"/displaypote\"},"
    "{\"text\":\"Display DHT\",\"callback_data\":\"/displaydht\"}]]";

WiFiClient clienteWifi;          // conexion que usa ThingSpeak

Actuador rele(PIN_RELE);         // el LED verde acompana al rele (ver setup)
Actuador ledAzul(PIN_LED_AZUL);
Potenciometro pote(PIN_POTE, UMBRAL_CRITICO_V);
SensorDHT dht(PIN_DHT);
PantallaOLED pantalla;
GestorWiFi wifi(WIFI_SSID, WIFI_PASS);
ClienteThingSpeak thingspeak(THINGSPEAK_CHANNEL_ID, THINGSPEAK_WRITE_KEY, ENVIO_THINGSPEAK_MS);
BotTelegram bot(TELEGRAM_BOT_TOKEN, TELEGRAM_CHAT_ID, CONSULTA_TELEGRAM_MS);
Temporizador lecturaPote(LECTURA_POTE_MS);
Temporizador lecturaDht(LECTURA_DHT_MS);
Temporizador envioThingSpeak(ENVIO_THINGSPEAK_MS);

// ---- prototipos de funciones ----
void atenderPotenciometro();
void atenderDHT();
void atenderThingSpeak();
void atenderTelegram();
void informarEstadoWifi();

void comandoStart();
void comandoClima();
void comandoRele(const String& comando);
void comandoThingUp();
void comandoDisplay(const String& componente);

void setup() {
    Serial.begin(BAUDIOS);
    rele.setIndicador(PIN_LED_VERDE);
    rele.begin();
    ledAzul.begin();
    pote.begin();
    dht.begin();

    if (!pantalla.begin()) {
        Serial.println("ERROR: no se encontro la pantalla OLED (0x3C)");
    }

    thingspeak.begin(clienteWifi);
    bot.begin();
    pantalla.mostrarConTitulo("WiFi", String("Conectando a\n") + wifi.ssid() + "...");

    wifi.begin();
    wifi.esperarConexion(ESPERA_WIFI_MS);   // unica espera bloqueante del programa, y con limite
    wifi.huboCambio();                      // se descarta el aviso: el estado inicial se informa aca
    informarEstadoWifi();

    lecturaPote.iniciar();
    lecturaDht.iniciar();
    envioThingSpeak.iniciar();
}

void loop() {
    rele.update();
    pantalla.update();

    wifi.update();
    if (wifi.huboCambio()) {
        informarEstadoWifi();
    }

    atenderPotenciometro();
    atenderDHT();
    atenderThingSpeak();
    atenderTelegram();
}

// ---- cuerpo de las funciones ----

// Cada ENVIO_THINGSPEAK_MS carga temperatura y humedad y pide enviarlas a ThingSpeak.
void atenderThingSpeak() {
    if (envioThingSpeak.cumplido() && dht.tieneDatos()) {
        thingspeak.setCampo(CAMPO_TEMPERATURA, dht.temperatura());
        thingspeak.setCampo(CAMPO_HUMEDAD, dht.humedad());
        thingspeak.solicitarEnvio();
    }

    thingspeak.update(wifi.estaConectado());

    if (thingspeak.huboResultado()) {
        int codigo = thingspeak.ultimoCodigo();
        Serial.printf("ThingSpeak: %s (codigo %d)\r\n",
                      ClienteThingSpeak::describirCodigo(codigo), codigo);
    }
}

// Recibe un comando de Telegram (si hay) y lo deriva a su funcion.
void atenderTelegram() {
    String comando;
    if (!wifi.estaConectado() || !bot.recibir(comando)) {
        return;
    }
    Serial.printf("Telegram: %s\r\n", comando.c_str());

    if (comando == "/start") {
        comandoStart();
    } else if (comando == "/clima") {
        comandoClima();
    } else if (comando.startsWith("/rele")) {
        comandoRele(comando);
    } else if (comando == "/thingup") {
        comandoThingUp();
    } else if (comando.startsWith("/display")) {
        comandoDisplay(comando.substring(strlen("/display")));
    } else {
        bot.enviar("Comando no reconocido. Enviá /start para ver las opciones.");
    }
}

void comandoStart() {
    bot.enviarConTeclado(
        "Hola! Elegí una opción o escribí un comando:\n"
        "/clima\n/releon <segundos>\n/releoff\n/thingup\n"
        "/displayled, /displaypote, /displaydht",
        TECLADO_INICIO);
}

void comandoClima() {
    if (!dht.tieneDatos()) {
        bot.enviar("Todavía no hay lecturas válidas del DHT22.");
        return;
    }
    bot.enviar("Temperatura: " + String(dht.temperatura(), 1) + " °C\n"
               "Humedad: " + String(dht.humedad(), 1) + " %");
}

// /releon <segundos> enciende el rele y se apaga solo. /releoff lo apaga.
void comandoRele(const String& comando) {
    if (comando == "/releoff") {
        rele.apagar();
        bot.enviar("Relé apagado.");
        return;
    }

    if (comando.startsWith("/releon ")) {
        long segundos = comando.substring(strlen("/releon ")).toInt();
        if (segundos > 0 && segundos <= RELE_MAX_SEGUNDOS) {
            rele.encenderPor(segundos * 1000UL);
            bot.enviar("Relé encendido por " + String(segundos) + " s.");
            return;
        }
    }

    bot.enviar("Uso: /releon <segundos> (1 a " + String(RELE_MAX_SEGUNDOS) + ") o /releoff");
}

// Envia el voltaje del potenciometro a ThingSpeak.
void comandoThingUp() {
    float voltaje = pote.voltaje();
    thingspeak.setCampo(CAMPO_VOLTAJE, voltaje);
    thingspeak.solicitarEnvio();

    String respuesta = "Voltaje: " + String(voltaje, 2) + " V. Se envía a ThingSpeak";
    unsigned long espera = thingspeak.msParaProximoEnvio() / 1000;
    if (espera > 0) {
        respuesta += " en " + String(espera) + " s";
    }
    respuesta += ".";
    if (pote.superaUmbral()) {
        respuesta += "\nSupera el umbral crítico de " + String(pote.umbral(), 2) + " V.";
    }
    bot.enviar(respuesta);
}

// Muestra en el OLED el estado del componente pedido (/displayled, /displaypote, /displaydht).
void comandoDisplay(const String& componente) {
    if (componente == "led") {
        pantalla.mostrarConTitulo("Estado LED",
            String("Led verde ") + rele.estadoTexto() + "\nLed azul " + ledAzul.estadoTexto());
    } else if (componente == "pote") {
        pantalla.mostrarConTitulo("Potenciometro",
            "Voltaje: " + String(pote.voltaje(), 2) + " V\n"
            "Umbral: " + String(pote.umbral(), 2) + " V\n" +
            (pote.superaUmbral() ? "Estado: ALERTA" : "Estado: normal"));
    } else if (componente == "dht") {
        if (dht.tieneDatos()) {
            pantalla.mostrarConTitulo("Sensor DHT22",
                "Temp: " + String(dht.temperatura(), 1) + " °C\n"
                "Humedad: " + String(dht.humedad(), 1) + " %");
        } else {
            pantalla.mostrarConTitulo("Sensor DHT22", "Sin datos validos");
        }
    } else {
        pantalla.mostrarConTitulo("Display", "Comando no identificado:\n/display" + componente);
        bot.enviar("Comando /display" + componente + " no identificado. Opciones: led, pote, dht.");
        return;
    }
    bot.enviar("Mostrando en el display: " + componente);
}

// Informa por Serial y en el OLED si el Wi-Fi esta conectado o no.
void informarEstadoWifi() {
    if (wifi.estaConectado()) {
        Serial.printf("WiFi conectado a %s | IP: %s | RSSI: %d dBm\r\n",
                      wifi.ssid(), wifi.ip().c_str(), wifi.rssi());
        pantalla.mostrarConTitulo("WiFi conectado",
            String("Red: ") + wifi.ssid() + "\nIP: " + wifi.ip(), 5000);
    } else {
        Serial.println("WiFi SIN conexion. Reintentando...");
        pantalla.mostrarConTitulo("WiFi", "Sin conexion\nReintentando...", 5000);
    }
}

// Lee el pote cada LECTURA_POTE_MS. Si cruza el umbral, avisa por Telegram sin que se lo pidan.
void atenderPotenciometro() {
    if (!lecturaPote.cumplido()) {
        return;
    }

    pote.update();

    if (pote.alertaNueva()) {
        String aviso = "ALERTA: el potenciómetro marca " + String(pote.voltaje(), 2) +
                       " V y supera el umbral de " + String(pote.umbral(), 2) + " V.";
        Serial.println(aviso);
        if (wifi.estaConectado()) {
            bot.enviar(aviso);
        }
    }

    // El LED refleja el estado real; el aviso (arriba) usa histeresis para no repetirse.
    if (pote.superaUmbral()) {
        ledAzul.encender();
    } else {
        ledAzul.apagar();
    }
}

// Lee el DHT22 cada LECTURA_DHT_MS. Si falla, conserva el ultimo valor valido.
void atenderDHT() {
    if (!lecturaDht.cumplido()) {
        return;
    }

    if (!dht.update()) {
        Serial.printf("DHT: lectura fallida (%u seguidas)\r\n", dht.fallosConsecutivos());
    }
}
