#include <Arduino.h>

// =================================================
// =============== PID BRAZO CON TRAYECTORIA =======
// =================================================

// ---- PINES ----
#define POT_BRAZO 35

#define IN3 25
#define IN4 26
#define ENB 27

// ---- LIMITES REALES ----
#define POT_MIN 10
#define POT_MAX 867
#define MARGEN  15

// ---- PWM ----
#define PWM_FREQ 1000
#define PWM_RES 8

#define PWM_KICK 150
#define PWM_MAX  220

// ---- PID ----
float Kp = 0.9;
float Ki = 0.0;
float Kd = 0.05;

float error = 0;
float errorPrev = 0;
float integral = 0;

// ---- TRAYECTORIA ----
int objetivo = POT_MIN + MARGEN;  // empieza abajo
int direccionTray = 1;            // +1 sube, -1 baja
#define PASO_TRAY 5                // tamaño del paso del objetivo

unsigned long tPrev = 0;
unsigned long tTrayPrev = 0;

// =================================================
// ================= SETUP =========================
// =================================================
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcAttach(ENB, PWM_FREQ, PWM_RES);

  tPrev = millis();
  tTrayPrev = millis();

  Serial.println("PID BRAZO CON TRAYECTORIA");
  Serial.println("Movimiento automatico entre limites");
}

// =================================================
// ================= LOOP ==========================
// =================================================
void loop() {

  // ---- LECTURA ----
  int posicion = analogRead(POT_BRAZO);

  // ---- TIEMPO PID ----
  unsigned long ahora = millis();
  float dt = (ahora - tPrev) / 1000.0;
  if (dt <= 0) dt = 0.01;
  tPrev = ahora;

  // ---- GENERADOR DE TRAYECTORIA ----
  if (ahora - tTrayPrev > 40) {  // velocidad del barrido
    objetivo += direccionTray * PASO_TRAY;

    if (objetivo >= POT_MAX - MARGEN) {
      objetivo = POT_MAX - MARGEN;
      direccionTray = -1;
    }

    if (objetivo <= POT_MIN + MARGEN) {
      objetivo = POT_MIN + MARGEN;
      direccionTray = 1;
    }

    tTrayPrev = ahora;
  }

  // ---- PID ----
  error = objetivo - posicion;
  float derivada = (error - errorPrev) / dt;
  errorPrev = error;

  // Zona muerta
  if (abs(error) < 8) {
    ledcWrite(ENB, 0);
    integral = 0;
    return;
  }

  float salida = Kp * error + Kd * derivada;

  int pwm = abs((int)salida);
  pwm = constrain(pwm, PWM_KICK, PWM_MAX);

  // ---- LIMITES DUROS ----
  if ((posicion <= POT_MIN + MARGEN && salida < 0) ||
      (posicion >= POT_MAX - MARGEN && salida > 0)) {
    ledcWrite(ENB, 0);
    return;
  }

  // ---- SENTIDO (TERCER CUADRANTE CORRECTO) ----
  if (salida > 0) {
    // SUBE (horario)
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  } else {
    // BAJA (antihorario)
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }

  ledcWrite(ENB, pwm);

  // ---- DEBUG ----
  Serial.print("Pos: ");
  Serial.print(posicion);
  Serial.print(" Obj: ");
  Serial.print(objetivo);
  Serial.print(" Dir: ");
  Serial.print(direccionTray);
  Serial.print(" PWM: ");
  Serial.println(pwm);

  delay(5);
}