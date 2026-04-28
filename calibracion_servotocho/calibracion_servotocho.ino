#include <Arduino.h>

// ================= SERVO 80 KG =================
#define SERVO_GRANDE_PIN 33
#define SERVO_FREQ 50
#define SERVO_RES 16

#define PASO_US 200       // ⬅️ PASO GRANDE (clave)
int servoPos = 1500;      // empezamos en el centro

uint32_t dutyFromUs(uint32_t us) {
  return (uint32_t)((us / 20000.0) * ((1 << SERVO_RES) - 1));
}

void setup() {
  Serial.begin(115200);

  Serial.println("CALIBRACION HOME SERVO 80KG");
  Serial.println("U -> subir");
  Serial.println("D -> bajar");
  Serial.println("H -> guardar HOME");

  ledcAttach(SERVO_GRANDE_PIN, SERVO_FREQ, SERVO_RES);
  ledcWrite(SERVO_GRANDE_PIN, dutyFromUs(servoPos));
}

void loop() {

  if (Serial.available()) {
    char c = Serial.read();

    if (c == 'U' || c == 'u') {
      servoPos -= PASO_US;
      servoPos = constrain(servoPos, 600, 2400);
      ledcWrite(SERVO_GRANDE_PIN, dutyFromUs(servoPos));
      Serial.print("Pos: ");
      Serial.println(servoPos);
    }

    if (c == 'D' || c == 'd') {
      servoPos += PASO_US;
      servoPos = constrain(servoPos, 600, 2400);
      ledcWrite(SERVO_GRANDE_PIN, dutyFromUs(servoPos));
      Serial.print("Pos: ");
      Serial.println(servoPos);
    }

    if (c == 'H' || c == 'h') {
      Serial.println("=== HOME GUARDADO ===");
      Serial.print("SERVO_GRANDE_HOME = ");
      Serial.println(servoPos);
    }
  }
}