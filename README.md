# TP Integrador - Grupo 6

ESP32 con DHT22, potenciómetro, LEDs, relé y OLED, conectada a ThingSpeak y a un bot de Telegram.

## Puesta en marcha

1. Copiar `include/secrets.h.example` como `include/secrets.h` y completar las credenciales.
2. Compilar con PlatformIO (`pio run`) y simular con la extensión Wokwi de VS Code.

Para obtener el `TELEGRAM_CHAT_ID`: escribirle al bot con el ID vacío; el monitor serie muestra
"mensaje ignorado del chat XXXX". Ese número es el ID. Para que lo use todo el grupo, agregar el bot
a un grupo de Telegram y usar el ID del grupo (es negativo).

## Comandos del bot

| Comando | Acción |
|---|---|
| `/start` | Muestra el teclado con botones |
| `/clima` | Última temperatura y humedad |
| `/releon <seg>` / `/releoff` | Enciende el relé por unos segundos / lo apaga |
| `/thingup` | Envía el voltaje del potenciómetro a ThingSpeak |
| `/displayled`, `/displaypote`, `/displaydht` | Muestra el estado en el OLED |

Si el potenciómetro supera 2.8 V, el bot envía una alerta automática.
