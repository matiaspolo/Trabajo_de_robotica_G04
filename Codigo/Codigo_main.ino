/*
 * TEST de motores - ESP32 + L298N
 * Compatible con Arduino IDE + ESP32 Core 3.x
 */

#define MOT1_EN   23
#define MOT1_IN1  18
#define MOT1_IN2  19

#define MOT2_EN   27
#define MOT2_IN3  25
#define MOT2_IN4  26

#define POT_PIN   34

// PWM
#define PWM_FREQ  5000
#define PWM_RES   8       // 0-255
#define MAX_PWM   127     // 50%

void stopAll() {
  ledcWrite(MOT1_EN, 0);
  ledcWrite(MOT2_EN, 0);

  digitalWrite(MOT1_IN1, LOW);
  digitalWrite(MOT1_IN2, LOW);
  digitalWrite(MOT2_IN3, LOW);
  digitalWrite(MOT2_IN4, LOW);

  Serial.println(">> MOTORES PARADOS");
}

void rampUp(uint8_t pwmPin, int targetPWM) {
  Serial.print("Rampa 0 -> ");
  Serial.println(targetPWM);

  for (int pwm = 0; pwm <= targetPWM; pwm += 5) {
    ledcWrite(pwmPin, pwm);
    delay(50);
  }

  ledcWrite(pwmPin, targetPWM);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=================================");
  Serial.println(" TEST DE MOTORES - ESP32 + L298N");
  Serial.println("=================================");
  Serial.println("1 = Motor1 adelante");
  Serial.println("2 = Motor2 adelante");
  Serial.println("3 = Motor1 atras");
  Serial.println("4 = Motor2 atras");
  Serial.println("0 = PARAR");
  Serial.println("p = Leer potenciometro");
  Serial.println("=================================");

  pinMode(MOT1_IN1, OUTPUT);
  pinMode(MOT1_IN2, OUTPUT);
  pinMode(MOT2_IN3, OUTPUT);
  pinMode(MOT2_IN4, OUTPUT);

  pinMode(POT_PIN, INPUT);
  analogReadResolution(12);

  // PWM ESP32 Core 3.x
  ledcAttach(MOT1_EN, PWM_FREQ, PWM_RES);
  ledcAttach(MOT2_EN, PWM_FREQ, PWM_RES);

  stopAll();
}

void loop() {

  if (Serial.available()) {

    char c = Serial.read();

    switch (c) {

      case '1':
        stopAll();
        delay(100);

        Serial.println(">> Motor 1 ADELANTE");

        digitalWrite(MOT1_IN1, HIGH);
        digitalWrite(MOT1_IN2, LOW);

        rampUp(MOT1_EN, MAX_PWM);
        break;

      case '2':
        stopAll();
        delay(100);

        Serial.println(">> Motor 2 ADELANTE");

        digitalWrite(MOT2_IN3, HIGH);
        digitalWrite(MOT2_IN4, LOW);

        rampUp(MOT2_EN, MAX_PWM);
        break;

      case '3':
        stopAll();
        delay(100);

        Serial.println(">> Motor 1 ATRAS");

        digitalWrite(MOT1_IN1, LOW);
        digitalWrite(MOT1_IN2, HIGH);

        rampUp(MOT1_EN, MAX_PWM);
        break;

      case '4':
        stopAll();
        delay(100);

        Serial.println(">> Motor 2 ATRAS");

        digitalWrite(MOT2_IN3, LOW);
        digitalWrite(MOT2_IN4, HIGH);

        rampUp(MOT2_EN, MAX_PWM);
        break;

      case '0':
        stopAll();
        break;

      case 'p':
      case 'P': {
        int val = analogRead(POT_PIN);

        Serial.print("Potenciometro: ");
        Serial.print(val);
        Serial.print(" / 4095 (");
        Serial.print((val * 100) / 4095);
        Serial.println("%)");
        break;
      }

      case '\n':
      case '\r':
        break;

      default:
        Serial.println("Comando no reconocido");
        break;
    }
  }

  delay(10);
}