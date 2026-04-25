// Código básico para comprobar conexión con ESP32 en Arduino IDE
// Abre el Monitor Serie a 115200 baudios

void setup() {
  Serial.begin(115200);
  delay(1000); // Espera inicial

  Serial.println("ESP32 conectado correctamente.");
  Serial.println("Prueba de comunicación serie OK.");
}

void loop() {
  Serial.println("ESP32 funcionando...");
  delay(2000);
}