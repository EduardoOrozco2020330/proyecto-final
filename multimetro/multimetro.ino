#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define WIDTH_OLED    128       // 128 pixeles de ancho 
#define HEIGHT_OLED   64        // 64 pixeles de alto
#define RESET_PIN     -1

Adafruit_SSD1306 oled_display(WIDTH_OLED, HEIGHT_OLED, &Wire, RESET_PIN);

// Define button pins
#define PIN_SUBIR 3
#define PIN_BAJAR 6
#define PIN_SELECCIONAR 5

// Variables for menu navigation
int opcionActual = 0;
int opcionMostrada = 0;
bool opcionSeleccionada = false;
unsigned long tiempoUltimoSubir = 0;
unsigned long tiempoUltimoBajar = 0;
unsigned long tiempoUltimoSeleccionar = 0;
const unsigned long debounceDelay = 50;

// Voltage and resistance calculations
float r1 = 100000;
float r2 = 20000;
int PinA0 = 0;
float Ve = 5; // Voltage in Arduino

// Current measurement variables
float sensibilidad = 0.66; // Sensitivity for 30 Amperes
float I = 0.00;
float ajuste = 0.05; // Adjust for calibration

void setup() {
    Serial.begin(9600); // Serial communication for Bluetooth
    // Initialize the OLED display
    if (!oled_display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("Error al inicializar la pantalla OLED"));
        for (;;);
    }
    oled_display.clearDisplay();
    oled_display.setTextSize(2);
    oled_display.setTextColor(SSD1306_WHITE);
    pinMode(PIN_SUBIR, INPUT);
    pinMode(PIN_BAJAR, INPUT);
    pinMode(PIN_SELECCIONAR, INPUT);
}

void loop() {
    oled_display.clearDisplay();
    oled_display.setCursor(0, 0);

    // Read button states
    if (digitalRead(PIN_SUBIR) == HIGH && (millis() - tiempoUltimoSubir > debounceDelay)) {
        tiempoUltimoSubir = millis();
        subirOpcion();
    }

    if (digitalRead(PIN_BAJAR) == HIGH && (millis() - tiempoUltimoBajar > debounceDelay)) {
        tiempoUltimoBajar = millis();
        bajarOpcion();
    }

    if (digitalRead(PIN_SELECCIONAR) == HIGH && (millis() - tiempoUltimoSeleccionar > debounceDelay)) {
        tiempoUltimoSeleccionar = millis();
        seleccionarOpcion();
    }

    if (!opcionSeleccionada) {
        switch (opcionMostrada) {
            case 0:
                oled_display.println("- Voltaje");
                oled_display.println("  Corrient");
                oled_display.println("  Resist");
                break;
            case 1:
                oled_display.println("  Voltaje");
                oled_display.println("- Corrient");
                oled_display.println("  Resist");
                break;
            case 2:
                oled_display.println("  Voltaje");
                oled_display.println("  Corrient");
                oled_display.println("- Resist");
                break;
        }
    } else {
        switch (opcionActual) {
            case 0: // Voltage
                float v = (analogRead(1) * 5) / 1023.0;
                float resultado = (v / (r2 / (r1 + r2)));
                oled_display.print(resultado);
                oled_display.print(" V");
                oled_display.println();
                
                // Enviar por Bluetooth
                Serial.print("Voltaje: ");
                Serial.print(resultado);
                Serial.println(" V");

                break;

            case 1: // Current
                I = promedio_I(500); // Average of 500 samples
                oled_display.print("I: ");
                oled_display.print(I >= 0.01 ? I : 0.00);
                oled_display.print(" A");
                oled_display.println();
                
                // Enviar por Bluetooth
                Serial.print("Corriente: ");
                Serial.print(I >= 0.01 ? I : 0.00);
                Serial.println(" A");

                break;

            case 2: // Resistance
                int lectura = analogRead(PinA0);
                if (lectura) {
                    float relacion = (lectura * Ve) / 1024.0;
                    float VR2 = relacion;
                    relacion = (Ve / VR2) - 1;
                    r2 = r1 * relacion;
                    if (r2 >= 20000) {
                        oled_display.println("Inserte correctamente");
                    } else if (r2 == 0) {
                        oled_display.println("Inserte resistencia");
                    } else {
                        oled_display.print("R2: ");
                        oled_display.println(r2);
                        
                        // Enviar por Bluetooth
                        Serial.print("Resistencia: ");
                        Serial.print(r2);
                        Serial.println(" Ohms");
                    }
                }
                break;
        }
    }

    oled_display.display();
    delay(100);
}

void subirOpcion() {
    if (!opcionSeleccionada) {
        opcionMostrada--;
        if (opcionMostrada < 0) {
            opcionMostrada = 2;
        }
    }
}

void bajarOpcion() {
    if (!opcionSeleccionada) {
        opcionMostrada++;
        if (opcionMostrada > 2) {
            opcionMostrada = 0;
        }
    }
}

void seleccionarOpcion() {
    if (!opcionSeleccionada) {
        opcionSeleccionada = true;
        opcionActual = opcionMostrada;
    } else {
        opcionSeleccionada = false;
    }
}

// Function to calculate the average current from 500 samples
float promedio_I(int muestras_I) {
    float sensorA0;
    float intencidad = 0;
    for (int i = 0; i < muestras_I; i++) {
        sensorA0 = analogRead(A0) * (5.0 / 1023.0);
        intencidad += (sensorA0 - 2.5) / sensibilidad; // Calculate current
    }
    return intencidad / muestras_I; // Average
}
