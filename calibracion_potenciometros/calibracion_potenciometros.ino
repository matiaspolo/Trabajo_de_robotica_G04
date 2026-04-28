#include <Arduino.h>

// ================= POTENCIOMETROS =================
#define POT_BASE  34
#define POT_BRAZO 35

int baseMin  = -1;
int baseMax  = -1;
int brazoMin = -1;
int brazoMax = -1;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12); // 0–4095

  Serial.println("=== CALIBRACION POTENCIOMETROS ===");
  Serial.println("Mueve a MANO cada eje y pulsa:");
  Serial.println("1 -> BASE MIN");
  Serial.println("2 -> BASE MAX");
  Serial.println("3 -> BRAZO MIN");
  Serial.println("4 -> BRAZO MAX");
}

void loop() {

  int base  = analogRead(POT_BASE);
  int brazo = analogRead(POT_BRAZO);

  Serial.print("BASE: ");
  Serial.print(base);
  Serial.print("   BRAZO: ");
  Serial.print(brazo);

  Serial.print("   | BMIN: ");
  Serial.print(baseMin);
  Serial.print(" BMAX: ");
  Serial.print(baseMax);
  Serial.print(" AMIN: ");
  Serial.print(brazoMin);
  Serial.print(" AMAX: ");
  Serial.println(brazoMax);

  if (Serial.available()) {
    char c = Serial.read();

    if (c == '1') {
      baseMin = base;
      Serial.print(">>> BASE MIN GUARDADO: ");
      Serial.println(baseMin);
    }

    if (c == '2') {
      baseMax = base;
      Serial.print(">>> BASE MAX GUARDADO: ");
      Serial.println(baseMax);
    }

    if (c == '3') {
      brazoMin = brazo;
      Serial.print(">>> BRAZO MIN GUARDADO: ");
      Serial.println(brazoMin);
    }

    if (c == '4') {
      brazoMax = brazo;
      Serial.print(">>> BRAZO MAX GUARDADO: ");
      Serial.println(brazoMax);
    }
  }

  delay(200);
}