#include <HardwareSerial.h>

// Vi bruker Serial2 på ESP32 (GPIO 16 for RX, GPIO 17 for TX)
HardwareSerial MySerial(2); 

void setup() {
  // Debug-melding til PC via USB
  Serial.begin(115200);
  while (!Serial);
  Serial.println("--- ESP32 UART Debug Start ---");

  // UART-kommunikasjon med Raspberry Pi (9600 baud)
  // Parametere: baud, config, RX_pin, TX_pin  (16,17)
  MySerial.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("Serial2 (UART) startet på GPIO 16(RX) og 17(TX)");
}

void loop() {
  // Send melding hvert 2. sekund
  MySerial.println("Hilsen fra ESP32 and test!");
  Serial.println("Sendt: 'Hilsen fra ESP32!'");

  // Sjekk om vi har mottatt noe fra Pi
  if (MySerial.available()) {
    String mottatt = MySerial.readStringUntil('\n');
    Serial.print("Mottatt fra Pi: ");
    Serial.println(mottatt);
  }
  
  delay(2000);
}