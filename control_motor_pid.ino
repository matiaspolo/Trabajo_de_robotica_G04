// ============================================================
// CONTROL PID DE POSICIÓN – Motor DC + Potenciómetro
// Plataforma: ESP32 · Arduino IDE
// ============================================================
//
// El potenciómetro mide la posición angular real del eje.
// El controlador PID calcula la señal PWM para llevar el
// motor a la consigna deseada (setpoint).
//
// Para ajustar las ganancias, abre el Serial Monitor/Plotter
// a 115200 baud y envía comandos:
//   s1500   → cambia setpoint a 1500 (rango 0-4095)
//   p2.5    → cambia Kp
//   i0.01   → cambia Ki
//   d0.8    → cambia Kd
// ============================================================

// ─── PINES ──────────────────────────────────────────────────
// Potenciómetro (feedback)
#define PIN_POT       34    // ADC1 – cursor del potenciómetro

// Motor DC (puente H – L298N o similar)
#define PIN_IN1       18
#define PIN_IN2       19
#define PIN_ENA       23    // PWM

// Servo carrusel de colores
#define PIN_SERVO     13

// Botones (INPUT_PULLUP → activo en LOW)
#define PIN_BTN_VERDE 14
#define PIN_BTN_ROJO  27
#define PIN_BTN_AZUL  26

// LEDs indicadores
#define PIN_LED_ROJO  4
#define PIN_LED_AZUL  16
#define PIN_LED_VERDE 17

// ─── PWM CONFIG (ESP32) ─────────────────────────────────────
#define PWM_FREQ      5000   // Hz
#define PWM_RES       8      // 8 bits → 0-255
#define PWM_CHANNEL   0

// ─── GANANCIAS PID (ajustar experimentalmente) ──────────────
float Kp = 1.0;
float Ki = 0.0;
float Kd = 0.0;

// ─── VARIABLES DE CONTROL ───────────────────────────────────
float setpoint      = 2048;  // Consigna inicial (mitad del rango ADC)
float posActual     = 0;
float error_val     = 0;
float errorPrev     = 0;
float integral      = 0;
float derivada      = 0;
float salida        = 0;

// Zona muerta: por debajo de este PWM el motor no se mueve
#define PWM_MIN       40
// Límite integral (anti-windup)
#define INTEGRAL_MAX  5000.0

// ─── TIEMPO ─────────────────────────────────────────────────
unsigned long tAnterior = 0;
#define DT_MS         10     // Periodo del lazo de control (ms)

// ─── FILTRO DE LECTURA ADC ──────────────────────────────────
#define N_MUESTRAS    8
int muestras[N_MUESTRAS];
int idxMuestra = 0;

// ─── TOLERANCIA (banda muerta de posición) ──────────────────
#define TOLERANCIA    15     // Unidades ADC (~1.3° en pot de 270°)


// =============================================================
// SETUP
// =============================================================
void setup() {
  Serial.begin(115200);
  Serial.println("== Control PID Motor DC ==");
  Serial.println("Comandos: s<valor> p<Kp> i<Ki> d<Kd>");

  // --- Pines motor ---
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);

  // PWM en el pin ENA (compatible con ESP32 Core 2.x y 3.x)
  #if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcAttach(PIN_ENA, PWM_FREQ, PWM_RES);
  #else
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES);
    ledcAttachPin(PIN_ENA, PWM_CHANNEL);
  #endif

  // --- Potenciómetro ---
  analogReadResolution(12);        // 0 – 4095
  analogSetAttenuation(ADC_11db);  // Rango 0 – 3.3V

  // Llenar buffer del filtro con lectura actual
  int lecturaInicial = analogRead(PIN_POT);
  for (int i = 0; i < N_MUESTRAS; i++) {
    muestras[i] = lecturaInicial;
  }

  // --- Botones ---
  pinMode(PIN_BTN_VERDE, INPUT_PULLUP);
  pinMode(PIN_BTN_ROJO,  INPUT_PULLUP);
  pinMode(PIN_BTN_AZUL,  INPUT_PULLUP);

  // --- LEDs ---
  pinMode(PIN_LED_ROJO,  OUTPUT);
  pinMode(PIN_LED_AZUL,  OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  digitalWrite(PIN_LED_ROJO,  LOW);
  digitalWrite(PIN_LED_AZUL,  LOW);
  digitalWrite(PIN_LED_VERDE, LOW);

  tAnterior = millis();
}


// =============================================================
// LOOP
// =============================================================
void loop() {
  unsigned long ahora = millis();

  // Ejecutar el lazo de control cada DT_MS milisegundos
  if (ahora - tAnterior >= DT_MS) {
    float dt = (ahora - tAnterior) / 1000.0;  // segundos
    tAnterior = ahora;

    // 1. LEER POSICIÓN (filtrada)
    posActual = leerPotFiltrado();

    // 2. CALCULAR ERROR
    error_val = setpoint - posActual;

    // 3. Si estamos dentro de la tolerancia → parar motor
    if (abs(error_val) < TOLERANCIA) {
      pararMotor();
      integral  = 0;
      errorPrev = 0;
      imprimirDatos(0);
      return;
    }

    // 4. TÉRMINO INTEGRAL (con anti-windup)
    integral += error_val * dt;
    integral = constrain(integral, -INTEGRAL_MAX, INTEGRAL_MAX);

    // 5. TÉRMINO DERIVATIVO
    derivada = (dt > 0) ? (error_val - errorPrev) / dt : 0;

    // 6. SALIDA PID
    salida = Kp * error_val + Ki * integral + Kd * derivada;

    // 7. APLICAR AL MOTOR
    aplicarMotor(salida);

    // 8. Guardar error anterior
    errorPrev = error_val;

    // 9. Imprimir para Serial Plotter
    imprimirDatos(salida);
  }

  // Procesar comandos por Serial
  procesarComandos();
}


// =============================================================
// FUNCIONES AUXILIARES
// =============================================================

// Lectura filtrada del potenciómetro (media móvil)
float leerPotFiltrado() {
  muestras[idxMuestra] = analogRead(PIN_POT);
  idxMuestra = (idxMuestra + 1) % N_MUESTRAS;

  long suma = 0;
  for (int i = 0; i < N_MUESTRAS; i++) {
    suma += muestras[i];
  }
  return (float)suma / N_MUESTRAS;
}

// Aplicar señal de control al motor
void aplicarMotor(float control) {
  // Dirección
  if (control > 0) {
    digitalWrite(PIN_IN1, HIGH);
    digitalWrite(PIN_IN2, LOW);
  } else {
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, HIGH);
  }

  // Mapear |control| a PWM (0-255)
  int pwm = (int)abs(control);

  // Zona muerta del motor
  if (pwm < PWM_MIN) {
    pwm = 0;
  }

  pwm = constrain(pwm, 0, 255);

  #if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(PIN_ENA, pwm);
  #else
    ledcWrite(PWM_CHANNEL, pwm);
  #endif
}

// Parar motor (freno por cortocircuito)
void pararMotor() {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);

  #if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(PIN_ENA, 0);
  #else
    ledcWrite(PWM_CHANNEL, 0);
  #endif
}

// Imprimir datos para Serial Plotter (formato CSV)
void imprimirDatos(float salidaPID) {
  Serial.print("Setpoint:");
  Serial.print(setpoint);
  Serial.print(",Posicion:");
  Serial.print(posActual);
  Serial.print(",Salida:");
  Serial.print(salidaPID);
  Serial.print(",Error:");
  Serial.println(error_val);
}

// Procesar comandos por Serial para ajuste en tiempo real
void procesarComandos() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() < 2) return;

    char tipo = cmd.charAt(0);
    float valor = cmd.substring(1).toFloat();

    switch (tipo) {
      case 's': case 'S':
        setpoint = constrain(valor, 0, 4095);
        Serial.print(">> Setpoint = ");
        Serial.println(setpoint);
        integral = 0;
        break;
      case 'p': case 'P':
        Kp = valor;
        Serial.print(">> Kp = ");
        Serial.println(Kp, 4);
        break;
      case 'i': case 'I':
        Ki = valor;
        integral = 0;
        Serial.print(">> Ki = ");
        Serial.println(Ki, 4);
        break;
      case 'd': case 'D':
        Kd = valor;
        Serial.print(">> Kd = ");
        Serial.println(Kd, 4);
        break;
      default:
        Serial.println(">> Comando no reconocido. Usa s, p, i, d");
        break;
    }
  }
}
