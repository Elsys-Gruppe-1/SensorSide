#include <EEPROM.h>
#include <OneWire.h>
#include <WiFi.h>        // Bibliotek for å håndtere WiFi-funksjoner på ESP32
#include <HTTPClient.h>   // Bibliotek for å sende HTTP-forespørsler

#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p); for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p) {int i = 0; byte *pp = (byte*)&(p); for(; i < sizeof(p); i++) pp[i] = EEPROM.read(address+i);}

#define SCOUNT 30
#define PH_SENSOR_PIN 33
#define TDS_SENSOR_PIN 32
#define DS18S20_Pin 2 // Temperature sensor pin
#define VREF 5.0 // ADC reference voltage (assuming 5V ADC)

// Definer SSID og passord til WiFi-nettverket du ønsker å koble til
const char* ssid = "Andreas";
const char* password = "12345678";

// Definer URL til Flask-serveren som ESP32 skal sende data til
const char* serverUrl = "http://192.168.229.19:5000/data";

// EEPROM addresses for pH calibration
#define SlopeValueAddress 0
#define InterceptValueAddress 4

OneWire ds(DS18S20_Pin);

int phAnalogBuffer[SCOUNT];
int tdsAnalogBuffer[SCOUNT];
int phBufferIndex = 0;
int tdsBufferIndex = 0;
float slopeValue, interceptValue, averagePhVoltage, tdsValue, temperature = 25;
boolean enterCalibrationFlag = false;

// Sampling time trackers for each sensor
unsigned long phSampleTimepoint = millis();
unsigned long tdsSampleTimepoint = millis();

void setup() {
    Serial.begin(115200);
    pinMode(2, INPUT_PULLUP);
    pinMode(PH_SENSOR_PIN, INPUT);
    pinMode(TDS_SENSOR_PIN, INPUT);
    readCharacteristicValues();

    // Koble til Wi-Fi
    WiFi.begin(ssid, password);

    // Vent til enheten er koblet til Wi-Fi-nettverket
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);  // Vent et sekund før neste forsøk
      Serial.println("Connecting to WiFi...");
      }
    Serial.println("Connected to WiFi");  // Skriv ut når enheten er tilkoblet
}

void loop() {
    // pH sensor reading every 40 ms
    if (millis() - phSampleTimepoint > 40U) {
        phSampleTimepoint = millis();
        phAnalogBuffer[phBufferIndex] = analogRead(PH_SENSOR_PIN);
        phBufferIndex = (phBufferIndex + 1) % SCOUNT;
        averagePhVoltage = getMedianNum(phAnalogBuffer, SCOUNT) * VREF / 1024.0;
    }

    // TDS sensor reading every 40 ms
    if (millis() - tdsSampleTimepoint > 40U) {
        tdsSampleTimepoint = millis();
        tdsAnalogBuffer[tdsBufferIndex] = analogRead(TDS_SENSOR_PIN);
        tdsBufferIndex = (tdsBufferIndex + 1) % SCOUNT;

        // TDS median calculation and compensation
        float tdsAverageVoltage = getMedianNum(tdsAnalogBuffer, SCOUNT) * VREF / 1024.0;
        float compensationCoefficient = 1.0 + 0.02 * (25 - 25.0); // Temp compensation formula
        float compensationVoltage = tdsAverageVoltage / compensationCoefficient; // Compensated voltage
        tdsValue = (133.42 * pow(compensationVoltage, 3) - 255.86 * pow(compensationVoltage, 2) + 857.39 * compensationVoltage) * 0.5;
    }

    // Temperature reading from DS18S20 sensor
    temperature = getTemp();

    // Print data every second
    static unsigned long printTimepoint = millis();
    if (millis() - printTimepoint > 1000U) {
        printTimepoint = millis();
        
        Serial.print("pH: ");
        Serial.print(averagePhVoltage / 1000.0 * slopeValue + interceptValue);
        Serial.print(" | Temp: ");
        Serial.print(temperature);
        Serial.print(" °C | TDS: ");
        Serial.print(tdsValue, 0);
        Serial.println(" ppm");


        // Sjekk om ESP32 fortsatt er koblet til Wi-Fi
        if (WiFi.status() == WL_CONNECTED) {
          HTTPClient http;  // Opprett en HTTPClient-objekt for å lage forespørselen
          http.begin(serverUrl);  // Sett opp URL-en for forespørselen
          http.addHeader("Content-Type", "application/json");  // Angi at vi sender JSON-data
            
          // Eksempel på data som skal sendes (her temperatur og fuktighet)
          String jsonData = "{\"temperature\": " + String(temperature, 0) + ", \"tdsverdi\": " + String(tdsValue, 0) + "}";

          // Send en POST-forespørsel med JSON-dataene
          int httpResponseCode = http.POST(jsonData);
            
        // Hvis forespørselen var vellykket (responskode > 0)
        if (httpResponseCode > 0) {
          String response = http.getString();  // Få responsen fra serveren
          Serial.println(httpResponseCode);    // Skriv ut HTTP-responskode
          Serial.println(response);            // Skriv ut responsinnholdet (hvis noe)
          } 
        else {
          // Hvis noe gikk galt med forespørselen
          Serial.println("Error sending request");
          }
          http.end();  // Lukk HTTP-forbindelsen
          }
    }
}

// Get temperature from DS18S20 sensor
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
    float tempRead = ((data[1] << 8) | data[0]) / 16.0;
    return tempRead;
}

// Median filter for stabilizing sensor readings
int getMedianNum(int bArray[], int iFilterLen) {
    int bTab[iFilterLen];
    memcpy(bTab, bArray, sizeof(bTab));
    for (int i = 0; i < iFilterLen - 1; i++) {
        for (int j = 0; j < iFilterLen - i - 1; j++) {
            if (bTab[j] > bTab[j + 1]) swap(bTab[j], bTab[j + 1]);
        }
    }
    return (iFilterLen % 2) ? bTab[iFilterLen / 2] : (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
}

// Load pH slope and intercept values from EEPROM
void readCharacteristicValues() {
    EEPROM_read(SlopeValueAddress, slopeValue);
    EEPROM_read(InterceptValueAddress, interceptValue);
    if (isnan(slopeValue) || isnan(interceptValue)) {
        slopeValue = 3.5;
        interceptValue = 0.0;
    }
}

// Utility swap function
void swap(int &a, int &b) {
    int temp = a;
    a = b;
    b = temp;
}
