// #include <OneWire.h>
// #include <HTTPClient.h>   // Bibliotek for å sende HTTP-forespørsler
// #include <HardwareSerial.h>
// #include <ArduinoJson.h>

// // Vi bruker Serial2 på ESP32 (GPIO 16 for RX, GPIO 17 for TX)
// HardwareSerial PiSerial(2); 

// #define SCOUNT 30
// #define TDS_SENSOR_PIN 32
// #define TEMP_Pin 2 // Temperature sensor pin (DS18S20)
// #define VREF 5.0 // ADC reference voltage (assuming 5V ADC)

// OneWire ds(TEMP_Pin);


// int tdsAnalogBuffer[SCOUNT];
// int tdsBufferIndex = 0;
// float slopeValue, interceptValue, averagePhVoltage, tdsValue, temperature = 25;
// boolean enterCalibrationFlag = false;

// // Sampling time trackers for each sensor
// unsigned long tdsSampleTimepoint = millis();

// void setup() {
//     Serial.begin(115200);
//     pinMode(TEMP_Pin, INPUT_PULLUP);
//     pinMode(TDS_SENSOR_PIN, INPUT);

//     while(!Serial); 
//     // UART serial interface using RX2 and TX2
//     PiSerial.begin(9600, SERIAL_8N1, 16, 17);
//     Serial.println("Serial2 (UART) startet på GPIO 16(RX) og 17(TX)");

// }

// void loop() {
//     // TDS sensor reading every 40 ms
//     if (millis() - tdsSampleTimepoint > 40U) {
//         tdsSampleTimepoint = millis();
//         tdsAnalogBuffer[tdsBufferIndex] = analogRead(TDS_SENSOR_PIN);
//         tdsBufferIndex = (tdsBufferIndex + 1) % SCOUNT;

//         // TDS median calculation and compensation
//         float tdsAverageVoltage = getMedianNum(tdsAnalogBuffer, SCOUNT) * VREF / 1024.0;
//         float compensationCoefficient = 1.0 + 0.02 * (25 - 25.0); // Temp compensation formula
//         float compensationVoltage = tdsAverageVoltage / compensationCoefficient; // Compensated voltage
//         tdsValue = (133.42 * pow(compensationVoltage, 3) - 255.86 * pow(compensationVoltage, 2) + 857.39 * compensationVoltage) * 0.5;
//     }

//     // Temperature reading from DS18S20 sensor
//     temperature = getTemp();

//     // Print/transmit data every second
//     static unsigned long printTimepoint = millis();
//     if (millis() - printTimepoint > 1000U) {
//         printTimepoint = millis();
//         sendData();
//         Serial.println();

//         // Sjekk om vi har mottatt noe fra Pi
//         if (PiSerial.available()) {
//             String mottatt = PiSerial.readStringUntil('\n');
//             Serial.print("Mottatt fra Pi: ");
//             Serial.println(mottatt);
//         }

//     }
// }

// // Get temperature from DS18S20 sensor
// float getTemp() {
//     byte data[12], addr[8];
//     if (!ds.search(addr)) {
//         ds.reset_search();
//         return -1000;
//     }
//     if (OneWire::crc8(addr, 7) != addr[7] || (addr[0] != 0x10 && addr[0] != 0x28)) return -1000;
//     ds.reset();
//     ds.select(addr);
//     ds.write(0x44, 1);
//     ds.reset();
//     ds.select(addr);
//     ds.write(0xBE);
//     for (int i = 0; i < 9; i++) data[i] = ds.read();
//     ds.reset_search();
//     float tempRead = ((data[1] << 8) | data[0]) / 16.0;
//     return tempRead;
// }

// // Median filter for stabilizing sensor readings
// int getMedianNum(int bArray[], int iFilterLen) {
//     int bTab[iFilterLen];
//     memcpy(bTab, bArray, sizeof(bTab));
//     for (int i = 0; i < iFilterLen - 1; i++) {
//         for (int j = 0; j < iFilterLen - i - 1; j++) {
//             if (bTab[j] > bTab[j + 1]) swap(bTab[j], bTab[j + 1]);
//         }
//     }
//     return (iFilterLen % 2) ? bTab[iFilterLen / 2] : (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
// }

// // Utility swap function
// void swap(int &a, int &b) {
//     int temp = a;
//     a = b;
//     b = temp;
// }

// void sendData() {
//     StaticJsonDocument<256> doc; // Økt litt for å få plass til mer data

//     doc["ts"] = millis()/1000; // Timestamp
//     doc["id"] = 1;

//     // Nested JSON
//     JsonObject sensorValues = doc.createNestedObject("SensorValues");
    
//     // Nå legger du verdier inn i det "nøstede" objektet
//     sensorValues["Temperature"] = temperature;
//     sensorValues["TDS"] = tdsValue;

//     // Multiple timestamps for sensor data
//     JsonObject saltData = doc.createNestedObject("Salt");
//     saltData["123.12312"] = 12.0;
//     saltData["123.13143"] = 12.2;
//     saltData["123.46542"] = 12.4;
    
//     String output;
//     serializeJson(doc, output);
//     PiSerial.println(output);
//     Serial.println(output);
// }



#include <OneWire.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h> // Lagt til for I2C

// ESP32 parameters
#define esp_nr 1
#define Depth esp_nr

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
    doc["ts"] = millis() / 1000;
    doc["esp_id"] = esp_nr;
    doc["pi_id"] = esp_nr;
    doc["d"] = Depth;   

    JsonObject sensorValues = doc.createNestedObject("Values");
    sensorValues["Tmp"] = temperature;
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