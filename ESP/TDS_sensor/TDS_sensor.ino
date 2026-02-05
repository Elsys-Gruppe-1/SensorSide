/***************************************************
DFRobot Gravity: Analog TDS Sensor / Meter For Arduino
https://www.dfrobot.com/wiki/index.php/Gravity:_Analog_TDS_Sensor
Created 2017-8-22
By Jason <jason.ling@dfrobot.com>
GNU Lesser General Public License
***************************************************/

// Constants and Pin Definitions
#define TdsSensorPin 33 // Define the pin where the TDS sensor is connected
#define VREF 5.0 // Reference voltage of the ADC (5V for Arduino Uno)
#define SCOUNT 30 // Number of samples to be taken for averaging

// Variables for Analog Data
int analogBuffer[SCOUNT]; // Array to store raw analog values
int analogBufferTemp[SCOUNT]; // Temporary array for sorting analog values
int analogBufferIndex = 0; // Index for filling analogBuffer
float averageVoltage = 0; // Average voltage from sensor readings
float tdsValue = 0; // Calculated TDS value (in ppm)
float temperature = 25; // Default temperature (used for temperature compensation)

// Setup function - Initializes serial communication and configures the TDS sensor pin
void setup() {
    Serial.begin(115200); // Start seria  l communication at 115200 bps
    pinMode(TdsSensorPin, INPUT); // Set the TDS sensor pin as input
}

// Main loop function - Runs continuously to read sensor data and calculate TDS
void loop() {
    // Read analog data from the sensor every 40 milliseconds
    static unsigned long analogSampleTimepoint = millis();
    if (millis() - analogSampleTimepoint > 40U) {
        analogSampleTimepoint = millis();
        analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin); // Read and store raw data
        analogBufferIndex++;
        if (analogBufferIndex == SCOUNT) analogBufferIndex = 0; // Reset index when full
    }

    // Calculate and print TDS every 800 milliseconds
    static unsigned long printTimepoint = millis();
    if (millis() - printTimepoint > 800U) {
        printTimepoint = millis();
        
        // Copy analogBuffer to a temporary array for median calculation
        for (int copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
            analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
        }

        // Get median voltage, convert to actual voltage, and apply temperature compensation
        averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;
        float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0); // Temp compensation formula
        float compensationVoltage = averageVoltage / compensationCoefficient; // Compensated voltage
        
        // Calculate TDS (in ppm) using a polynomial relationship
        tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage 
                    - 255.86 * compensationVoltage * compensationVoltage 
                    + 857.39 * compensationVoltage) * 0.5;
        
        // Output TDS value to serial monitor
        Serial.print("TDS Value: ");
        Serial.print(tdsValue, 0); // Print TDS value rounded to nearest integer
        Serial.println(" ppm");
    }
}

// Function to calculate median from an array (for noise reduction)
int getMedianNum(int bArray[], int iFilterLen) {
    int bTab[iFilterLen];
    for (int i = 0; i < iFilterLen; i++) {
        bTab[i] = bArray[i];
    }
    int temp;
    for (int j = 0; j < iFilterLen - 1; j++) {
        for (int i = 0; i < iFilterLen - j - 1; i++) {
            if (bTab[i] > bTab[i + 1]) {
                temp = bTab[i];
                bTab[i] = bTab[i + 1];
                bTab[i + 1] = temp;
            }
        }
    }
    if (iFilterLen % 2 == 1) return bTab[iFilterLen / 2]; // Odd number of elements
    else return (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2; // Even number of elements
}


