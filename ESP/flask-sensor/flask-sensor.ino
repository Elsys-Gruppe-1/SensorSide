
#include <WiFi.h>        // Bibliotek for å håndtere WiFi-funksjoner på ESP32
#include <HTTPClient.h>   // Bibliotek for å sende HTTP-forespørsler


#define TdsSensorPin 32
#define VREF 5.0 // analog reference voltage (Volt) of the ADC
#define SCOUNT 30 // sum of sample points
int analogBuffer[SCOUNT]; // array to store analog values
int analogBufferTemp[SCOUNT]; // temporary array for processing
int analogBufferIndex = 0; // index for the analog buffer
float temperature = 25; // assumed temperature in Celsius


// Definer SSID og passord til WiFi-nettverket du ønsker å koble til
const char* ssid = "Andreas";
const char* password = "12345678";

// Definer URL til Flask-serveren som ESP32 skal sende data til
const char* serverUrl = "http://192.168.9.177:5005/data";

void setup() {
  Serial.begin(115200);  // Start serielt grensesnitt for debugging
  pinMode(TdsSensorPin, INPUT);
  // Koble til Wi-Fi
  
  Serial.println("Wi-Fi status: " + String(WiFi.status()));
  WiFi.begin(ssid, password);
  // Vent til enheten er koblet til Wi-Fi-nettverket
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);  // Vent et sekund før neste forsøk
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");  // Skriv ut når enheten er tilkoblet
}


// Function to calculate and return TDS value
float tdsVerdi() {
    float averageVoltage = 0, tdsValue = 0; // local variables for voltage and TDS value

    // Read analog values into buffer
    for (int i = 0; i < SCOUNT; i++) {
        analogBuffer[i] = analogRead(TdsSensorPin); // read the analog value

        delay(40); // delay to allow for stable readings
    }

    // Calculate average voltage from the buffer
    averageVoltage = getMedianNum(analogBuffer, SCOUNT) * (float)VREF / 1024.0; // find median and convert to voltage
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0); // temperature compensation
    float compensationVoltage = averageVoltage / compensationCoefficient; // adjust voltage for temperature
    tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 
                255.86 * compensationVoltage * compensationVoltage + 
                857.39 * compensationVoltage); // calculate TDS value

    return tdsValue; // return calculated TDS value
}



void loop() {
  float currentTdsValue = tdsVerdi();
  Serial.println(currentTdsValue,0);
  // Sjekk om ESP32 fortsatt er koblet til Wi-Fi
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;  // Opprett en HTTPClient-objekt for å lage forespørselen
    http.begin(serverUrl);  // Sett opp URL-en for forespørselen
    http.addHeader("Content-Type", "application/json");  // Angi at vi sender JSON-data

    // Eksempel på data som skal sendes (her temperatur og fuktighet)
    String jsonData = "{\"temperature\": 26, \"tdsverdi\": "+ String(currentTdsValue, 0) +"}";

    // Send en POST-forespørsel med JSON-dataene
    int httpResponseCode = http.POST(jsonData);

    // Hvis forespørselen var vellykket (responskode > 0)
    if (httpResponseCode > 0) {
      String response = http.getString();  // Få responsen fra serveren
      Serial.println(httpResponseCode);    // Skriv ut HTTP-responskode
      Serial.println(response);            // Skriv ut responsinnholdet (hvis noe)
    } else {
      // Hvis noe gikk galt med forespørselen
      Serial.println("Error sending request");
    }
    
    http.end();  // Lukk HTTP-forbindelsen
  }
  
  delay(1000);  // Vent 10 sekunder før neste forespørsel
}



int getMedianNum(int bArray[], int iFilterLen) {
    int bTab[iFilterLen];
    for (byte i = 0; i < iFilterLen; i++)
        bTab[i] = bArray[i]; // copy the array
    
    // Sort the array using a simple bubble sort
    for (int j = 0; j < iFilterLen - 1; j++) {
        for (int i = 0; i < iFilterLen - j - 1; i++) {
            if (bTab[i] > bTab[i + 1]) {
                int bTemp = bTab[i];
                bTab[i] = bTab[i + 1];
                bTab[i + 1] = bTemp;
            }
        }
    }

    // Calculate median value
    int bTemp;
    if ((iFilterLen & 1) > 0) // if odd number of elements
        bTemp = bTab[(iFilterLen - 1) / 2];
    else // if even number of elements
        bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;

    return bTemp; // return median value
}



