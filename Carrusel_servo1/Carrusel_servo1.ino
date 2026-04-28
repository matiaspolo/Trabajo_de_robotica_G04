#include <ESP32Servo.h>

// ===== BOTONES =====
#define BOTON_VERDE 14
#define BOTON_ROJO  32
#define BOTON_AZUL  15

// ===== LEDS NORMALES =====
#define LED_ROJO   4
#define LED_AZUL   16
#define LED_VERDE  17

// ===== SERVO CARRUSEL =====
#define SERVO_PIN 13
Servo servoCarrusel;

// ===== POSICIONES DEL SERVO (±60) =====
#define POS_VERDE  30
#define POS_ROJO   90
#define POS_AZUL   150

void setup() {
  // Botones
  pinMode(BOTON_VERDE, INPUT_PULLUP);
  pinMode(BOTON_ROJO,  INPUT_PULLUP);
  pinMode(BOTON_AZUL,  INPUT_PULLUP);

  // LEDs
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);

  // Servo
  servoCarrusel.attach(SERVO_PIN);

  // Apagar todos los LEDs
  apagarLEDs();

  // Estado inicial: ROJO
  servoCarrusel.write(POS_ROJO);
  encenderRojo();
}

void loop() {

  if (digitalRead(BOTON_VERDE) == LOW) {
    servoCarrusel.write(POS_VERDE);
    encenderVerde();
    delay(300);
  }

  if (digitalRead(BOTON_ROJO) == LOW) {
    servoCarrusel.write(POS_ROJO);
    encenderRojo();
    delay(300);
  }

  if (digitalRead(BOTON_AZUL) == LOW) {
    servoCarrusel.write(POS_AZUL);
    encenderAzul();
    delay(300);
  }
}

// ===== FUNCIONES LEDS =====

void apagarLEDs() {
  digitalWrite(LED_ROJO, LOW);
  digitalWrite(LED_AZUL, LOW);
  digitalWrite(LED_VERDE, LOW);
}

void encenderRojo() {
  apagarLEDs();
  digitalWrite(LED_ROJO, HIGH);
}

void encenderVerde() {
  apagarLEDs();
  digitalWrite(LED_VERDE, HIGH);
}

void encenderAzul() {
  apagarLEDs();
  digitalWrite(LED_AZUL, HIGH);
}
