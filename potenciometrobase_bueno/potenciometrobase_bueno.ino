#include <Arduino.h>

/*************************************************
 * PID BASE CON TRAYECTORIA CORRECTA
 * 2820 -> 1558 -> 2820
 * GANANCIA NEGATIVA (SENTIDO CORRECTO)
 *************************************************/

// ================= PINES =================
#define POT_BASE 34

#define IN1 18
#define IN2 19
#define ENA 23

// ================= LIMITES =================
#define POS_MAX 3000
#define POS_MIN 1800
#define MARGEN  20

// ================= PWM =================
#define PWM_FREQ 1000
#define PWM_RES 8
#define PWM_KICK 170
#define PWM_MAX  230

// ================= PID =================
float Kp = 1.4;
float Ki = 0.0;
float Kd = 0.10;

float error = 0;
float errorPrev = 0;

// ================= TRAYECTORIA =================
int objetivo = POS_MAX;   // empieza arriba
int dirTray  = -1;        // primero baja

#define PASO_OBJ 6
#define DT_TRAY  40

unsigned long tPrev = 0;
unsigned long tTrayPrev = 0;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  ledcAttach(ENA, PWM_FREQ, PWM_RES);

  tPrev = millis();
  tTrayPrev = millis();

  Serial.println("PID BASE 2820 <-> 1558 (CORREGIDO)");
}

void loop() {

  int posicion = analogRead(POT_BASE);

  unsigned long ahora = millis();
  float dt = (ahora - tPrev) / 1000.0;
  if (dt <= 0) dt = 0.01;
  tPrev = ahora;

  // -------- TRAYECTORIA --------
  if (ahora - tTrayPrev > DT_TRAY) {

    objetivo += dirTray * PASO_OBJ;

    if (objetivo <= POS_MIN) {
      objetivo = POS_MIN;
      dirTray = 1;
    }

    if (objetivo >= POS_MAX) {
      objetivo = POS_MAX;
      dirTray = -1;
    }

    tTrayPrev = ahora;
  }

  // -------- PID (CAMBIO CLAVE AQUÍ) --------
  error = posicion - objetivo;   // ⬅️ ESTE ES EL CAMBIO

  float derivada = (error - errorPrev) / dt;
  errorPrev = error;

  if (abs(error) < 5) {
    ledcWrite(ENA, 0);
    return;
  }

  float salida = Kp * error + Kd * derivada;

  int pwm = abs((int)salida);
  pwm = constrain(pwm, PWM_KICK, PWM_MAX);

  // -------- SENTIDO --------
  if (salida > 0) {
    // ANTIHORARIO: 2820 -> 1558
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  } else {
    // HORARIO: 1558 -> 2820
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  }

  ledcWrite(ENA, pwm);

  // -------- DEBUG --------
  Serial.print("Pos: ");
  Serial.print(posicion);
  Serial.print(" Obj: ");
  Serial.print(objetivo);
  Serial.print(" Err: ");
  Serial.print(error);
  Serial.print(" PWM: ");
  Serial.println(pwm);

  delay(5);
}