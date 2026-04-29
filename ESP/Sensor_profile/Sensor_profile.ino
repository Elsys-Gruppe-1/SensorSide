#include <OneWire.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h> // Lagt til for I2C

// ESP32 parameters
#define esp_nr 1
#define Depth esp_nr*0.5

// I2C Innstillinger
#define I2C_DEV_ADDR 0x07 + esp_nr// ESP32 sin adresse på I2C-bussen
#define SDA_PIN 21        // Standard SDA på ESP32
#define SCL_PIN 22        // Standard SCL på ESP32

#define SCOUNT 30
#define TDS_SENSOR_PIN 32
#define TEMP_Pin 4 // Temperature sensor pin (DS18S20)
#define VREF 5.0   // ADC reference voltage

OneWire ds(TEMP_Pin);

int tdsAnalogBuffer[SCOUNT];
int tdsBufferIndex = 0;
float tdsValue, temperature = 25;
String lastJsonOutput = ""; // Lagrer siste JSON-streng for I2C-forespørsler

unsigned long tdsSampleTimepoint = millis();

// Funksjon som kjører når I2C Master (Pi) ber om data
void onRequest() {
    // Vi må sende pekeren til strengen OG lengden på strengen
    Wire.write((const uint8_t*)lastJsonOutput.c_str(), lastJsonOutput.length());
}

// Funksjon som kjører når I2C Master sender data til ESP32
void onReceive(int len) {
    String mottatt = "";
    while (Wire.available()) {
        mottatt += (char)Wire.read();
    }
    Serial.print("Mottatt fra Pi via I2C: ");
    Serial.println(mottatt);
}

void setup() {
    Serial.begin(115200);
    pinMode(TEMP_Pin, INPUT_PULLUP);
    pinMode(TDS_SENSOR_PIN, INPUT);

    // Initialiser I2C som Slave
    Wire.begin(I2C_DEV_ADDR, SDA_PIN, SCL_PIN, 400000); 
    Wire.onRequest(onRequest);
    Wire.onReceive(onReceive);

    Serial.printf("I2C Slave startet på adresse 0x%02X\n", I2C_DEV_ADDR);
}

void loop() {
    // TDS måling
    if (millis() - tdsSampleTimepoint > 40U) {
        tdsSampleTimepoint = millis();
        tdsAnalogBuffer[tdsBufferIndex] = analogRead(TDS_SENSOR_PIN);
        tdsBufferIndex = (tdsBufferIndex + 1) % SCOUNT;

        float tdsAverageVoltage = getMedianNum(tdsAnalogBuffer, SCOUNT) * VREF / 4095.0; // ESP32 bruker 12-bit ADC (4095)
        float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0); 
        float compensationVoltage = tdsAverageVoltage / compensationCoefficient;
        tdsValue = (133.42 * pow(compensationVoltage, 3) - 255.86 * pow(compensationVoltage, 2) + 857.39 * compensationVoltage) * 0.5;
    }

    temperature = getTemp();

    static unsigned long updateTimepoint = millis();
    if (millis() - updateTimepoint > 1000U) {
        updateTimepoint = millis();
        prepareData(); // Lager klar JSON-pakken
        Serial.println("Data oppdatert for I2C: " + lastJsonOutput);
    }
}

// Samler data i en JSON-streng som lagres i 'lastJsonOutput'
void prepareData() {
    StaticJsonDocument<256> doc;
    doc["pi_id"] = 1;
    doc["depth"] = (double)Depth;   

    JsonObject sensorValues = doc.createNestedObject("sensor_value");
    sensorValues["Temperatur"] = temperature;
    sensorValues["TDS"] = tdsValue;

    serializeJson(doc, lastJsonOutput);
}

// --- Hjelpefunksjoner (Temperatur og Median) ---

float getTemp() {
    byte data[12], addr[8];
    if (!ds.search(addr)) {
        ds.reset_search();
        return -1000;
    }
    if (OneWire::crc8(addr, 7) != addr[7] || (addr[0] != 0x10 && addr[0] != 0x28)) return -1000;
    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1);
    ds.reset();
    ds.select(addr);
    ds.write(0xBE);
    for (int i = 0; i < 9; i++) data[i] = ds.read();
    ds.reset_search();
    return ((data[1] << 8) | data[0]) / 16.0;
}

int getMedianNum(int bArray[], int iFilterLen) {
    int bTab[iFilterLen];
    memcpy(bTab, bArray, sizeof(bTab));
    for (int i = 0; i < iFilterLen - 1; i++) {
        for (int j = 0; j < iFilterLen - i - 1; j++) {
            if (bTab[j] > bTab[j + 1]) {
                int temp = bTab[j];
                bTab[j] = bTab[j + 1];
                bTab[j + 1] = temp;
            }
        }
    }
    return (iFilterLen % 2) ? bTab[iFilterLen / 2] : (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
}   