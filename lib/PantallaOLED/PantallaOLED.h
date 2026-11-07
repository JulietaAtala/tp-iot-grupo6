#ifndef PANTALLA_OLED_H
#define PANTALLA_OLED_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SH110X.h>
#include <Temporizador.h>

/**
 * Pantalla OLED I2C (128x64, driver SH1106G) para mostrar mensajes de estado.
 *
 * Resuelve los problemas tipicos de mostrar texto en un OLED:
 *  - Ancho fijo: con la fuente por defecto entran 21 caracteres por linea y 8 lineas.
 *    El texto largo se parte en lineas (sin cortar palabras si es posible) y, si no entra
 *    todo, se recorta y termina en "...".
 *  - Tildes, enie y simbolo de grado: la fuente por defecto no tiene tildes y los textos
 *    de Telegram llegan en UTF-8. Se transforman (a, e, i, o, u, n) y el grado se dibuja
 *    con el glifo CP437. Cualquier otro simbolo se reemplaza por '?'.
 *  - Mensajes temporales: con duracionMs > 0 el mensaje se borra solo (llamar a update()).
 */
class PantallaOLED {
public:
    static const int COLUMNAS = 21;          // caracteres por linea
    static const int FILAS = 8;              // lineas sin titulo
    static const int FILAS_CON_TITULO = 6;   // lineas de cuerpo cuando hay titulo

    explicit PantallaOLED(uint8_t direccionI2C = 0x3C);

    bool begin();                            // false si no se encuentra la pantalla
    void update();                           // atiende el borrado automatico
    void limpiar();

    // Texto libre, con salto de linea automatico. duracionMs = 0: queda hasta el proximo mensaje.
    void mostrarMensaje(const String& texto, unsigned long duracionMs = 0);

    // Titulo (una linea) + raya + cuerpo con salto de linea automatico.
    void mostrarConTitulo(const String& titulo, const String& cuerpo, unsigned long duracionMs = 0);

    // Utilidades (publicas para poder probarlas por separado)
    static String normalizar(const String& texto);                                  // UTF-8 -> texto apto OLED
    static int ajustarTexto(const String& texto, String lineas[], int maxLineas);   // devuelve cantidad TOTAL de lineas

private:
    void mostrar(const String& titulo, bool hayTitulo, const String& cuerpo, unsigned long duracionMs);

    Adafruit_SH1106G display;
    uint8_t direccion;
    bool disponible;
    Temporizador temporizador;               // one-shot para el borrado automatico
};

#endif