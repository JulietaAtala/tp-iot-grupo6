#include "PantallaOLED.h"

namespace {

const int ANCHO_PX = 128;
const int ALTO_LINEA_PX = 8;
const char GRADO_CP437 = (char)248;    // glifo del simbolo de grado en la fuente por defecto

// Traduce un caracter UTF-8 de 2 bytes (c, c2) al caracter mas parecido que dibuja la fuente.
char traducirUtf8(uint8_t c, uint8_t c2) {
    if (c == 0xC3) {
        switch (c2) {
            case 0xA1: case 0xA0: case 0xA4: return 'a';
            case 0xA9: case 0xA8: case 0xAB: return 'e';
            case 0xAD: case 0xAC: case 0xAF: return 'i';
            case 0xB3: case 0xB2: case 0xB6: return 'o';
            case 0xBA: case 0xB9: case 0xBC: return 'u';
            case 0xB1: return 'n';
            case 0x81: case 0x80: case 0x84: return 'A';
            case 0x89: case 0x88: case 0x8B: return 'E';
            case 0x8D: case 0x8C: case 0x8F: return 'I';
            case 0x93: case 0x92: case 0x96: return 'O';
            case 0x9A: case 0x99: case 0x9C: return 'U';
            case 0x91: return 'N';
        }
    } else if (c == 0xC2) {
        if (c2 == 0xB0) return GRADO_CP437;   // °
        if (c2 == 0xA1) return '!';           // ¡
        if (c2 == 0xBF) return '?';           // ¿
    }
    return '?';                               // cualquier otro simbolo
}

}  // namespace

PantallaOLED::PantallaOLED(uint8_t direccionI2C)
    : display(ANCHO_PX, 64, &Wire, -1), direccion(direccionI2C),
      disponible(false), temporizador(0, false) {}

bool PantallaOLED::begin() {
    disponible = display.begin(direccion, true);
    if (disponible) {
        display.cp437(true);                  // para dibujar el simbolo de grado
        display.setTextSize(1);
        display.setTextColor(SH110X_WHITE);
        limpiar();
    }
    return disponible;
}

void PantallaOLED::update() {
    if (temporizador.cumplido()) {
        limpiar();                            // vencio el mensaje temporal
    }
}

void PantallaOLED::limpiar() {
    if (!disponible) {
        return;
    }
    temporizador.detener();
    display.clearDisplay();
    display.display();
}

void PantallaOLED::mostrarMensaje(const String& texto, unsigned long duracionMs) {
    mostrar("", false, texto, duracionMs);
}

void PantallaOLED::mostrarConTitulo(const String& titulo, const String& cuerpo, unsigned long duracionMs) {
    mostrar(titulo, true, cuerpo, duracionMs);
}

// Convierte texto UTF-8 en texto que la fuente del OLED puede dibujar.
String PantallaOLED::normalizar(const String& texto) {
    String salida = "";
    unsigned int i = 0;
    unsigned int n = texto.length();

    while (i < n) {
        uint8_t c = (uint8_t)texto[i];
        if (c < 0x80) {                                   // ASCII
            if (c == '\n') {
                salida += '\n';
            } else if (c == '\t') {
                salida += ' ';
            } else if (c >= 32 && c < 127) {
                salida += (char)c;
            }                                             // otros caracteres de control: se descartan
            i++;
        } else {                                          // secuencia multibyte UTF-8
            int extra = (c >= 0xF0) ? 3 : (c >= 0xE0) ? 2 : (c >= 0xC0) ? 1 : 0;
            uint8_t c2 = (extra >= 1 && i + 1 < n) ? (uint8_t)texto[i + 1] : 0;
            salida += traducirUtf8(c, c2);
            i += 1 + extra;                               // una sola '?' por simbolo, aunque ocupe varios bytes
        }
    }
    return salida;
}

// Parte el texto en lineas de hasta COLUMNAS caracteres, sin cortar palabras.
// Respeta los '\n'. Llena como maximo maxLineas posiciones de 'lineas' y devuelve
// la cantidad TOTAL de lineas que hacian falta (si es mayor a maxLineas, no entraba todo).
int PantallaOLED::ajustarTexto(const String& texto, String lineas[], int maxLineas) {
    int total = 0;
    String linea = "";
    String palabra = "";

    auto cerrarLinea = [&]() {
        if (total < maxLineas) {
            lineas[total] = linea;
        }
        total++;
        linea = "";
    };

    auto agregarPalabra = [&]() {
        // Una palabra mas larga que una linea entera se corta en pedazos.
        while ((int)palabra.length() > COLUMNAS) {
            if (linea.length() > 0) {
                cerrarLinea();
            }
            if (total < maxLineas) {
                lineas[total] = palabra.substring(0, COLUMNAS);
            }
            total++;
            palabra = palabra.substring(COLUMNAS);
        }
        if (palabra.length() == 0) {
            return;
        }
        int necesario = linea.length() + (linea.length() > 0 ? 1 : 0) + palabra.length();
        if (necesario > COLUMNAS) {
            cerrarLinea();
        }
        if (linea.length() > 0) {
            linea += ' ';
        }
        linea += palabra;
        palabra = "";
    };

    for (unsigned int i = 0; i < texto.length(); i++) {
        char c = texto[i];
        if (c == '\n') {
            agregarPalabra();
            cerrarLinea();
        } else if (c == ' ') {
            agregarPalabra();
        } else {
            palabra += c;
        }
    }
    agregarPalabra();
    if (linea.length() > 0) {
        cerrarLinea();
    }
    return total;
}

void PantallaOLED::mostrar(const String& titulo, bool hayTitulo, const String& cuerpo, unsigned long duracionMs) {
    if (!disponible) {
        return;
    }

    int maxFilas = hayTitulo ? FILAS_CON_TITULO : FILAS;
    String lineas[FILAS];
    int total = ajustarTexto(normalizar(cuerpo), lineas, maxFilas);
    int n = total;

    if (total > maxFilas) {                   // no entra todo: se recorta y se avisa con "..."
        n = maxFilas;
        lineas[maxFilas - 1] = lineas[maxFilas - 1].substring(0, COLUMNAS - 3) + "...";
    }

    display.clearDisplay();

    int y = 0;
    if (hayTitulo) {
        String t = normalizar(titulo);
        if ((int)t.length() > COLUMNAS) {
            t = t.substring(0, COLUMNAS);
        }
        display.setCursor(0, 0);
        display.print(t);
        display.drawFastHLine(0, 9, ANCHO_PX, SH110X_WHITE);
        y = 12;
    }

    for (int i = 0; i < n; i++) {
        display.setCursor(0, y + i * ALTO_LINEA_PX);
        display.print(lineas[i]);
    }
    display.display();

    if (duracionMs > 0) {
        temporizador.setIntervalo(duracionMs);
        temporizador.iniciar();
    } else {
        temporizador.detener();               // mensaje permanente
    }
}