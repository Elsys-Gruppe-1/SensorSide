  // Include library to handle EEPROM
#include <EEPROM.h>

// Macros for writing and reading data from EEPROM (persistent memory)
#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p); for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p) {int i = 0; byte *pp = (byte*)&(p); for(; i < sizeof(p); i++) pp[i] = EEPROM.read(address+i);}

// Constants
#define ReceivedBufferLength 20 // Max length of received commands via Serial
#define SCOUNT 30               // Number of samples for voltage averaging
#define SlopeValueAddress 0      // EEPROM address for saving slope
#define InterceptValueAddress 4  // EEPROM address for saving intercept
#define SensorPin 33             // Analog pin for the pH sensor
#define VREF 5000                // Reference voltage in millivolts

// Global Variables
char receivedBuffer[ReceivedBufferLength + 1]; // Buffer for serial input
byte receivedBufferIndex = 0; // Tracks position in receivedBuffer
int analogBuffer[SCOUNT];     // Buffer for storing sensor readings
int analogBufferIndex = 0;    // Index for current position in analogBuffer
float slopeValue, interceptValue, averageVoltage; // Calibration values
boolean enterCalibrationFlag = 0;  // Tracks whether we are in calibration mode

void setup() {
    Serial.begin(115200);  // Begin serial communication at 115200 baud
    readCharacteristicValues(); // Read slope and intercept from EEPROM
}

void loop() {
    // If serial data is available, handle calibration commands
    if (serialDataAvailable()) {
        byte modeIndex = uartParse(); // Parse the received command
        phCalibration(modeIndex);     // Perform calibration if needed
        // Update slope and intercept from EEPROM after calibration
        EEPROM_read(SlopeValueAddress, slopeValue);
        EEPROM_read(InterceptValueAddress, interceptValue);
    }

    // Sample sensor voltage every 40 milliseconds
    static unsigned long sampleTimepoint = millis();
    if (millis() - sampleTimepoint > 40U) {
        sampleTimepoint = millis();
        analogBuffer[analogBufferIndex] = analogRead(SensorPin) / 1024.0 * VREF;
        analogBufferIndex++;
        if (analogBufferIndex == SCOUNT) analogBufferIndex = 0;
        averageVoltage = getMedianNum(analogBuffer, SCOUNT); // Get stable voltage
    }

    // Print data every second
    static unsigned long printTimepoint = millis();
    if (millis() - printTimepoint > 1000U) {
        printTimepoint = millis();
        if (enterCalibrationFlag) {
            Serial.print("Voltage: ");
            Serial.print(averageVoltage);
            Serial.println(" mV");  // Print voltage during calibration
        } else {
            Serial.print("pH: ");
            Serial.println(averageVoltage / 1000.0 * slopeValue + interceptValue);  // Print pH value during normal operation
        }
    }
}

// Check if serial data is available
boolean serialDataAvailable(void) {
    char receivedChar;
    static unsigned long receivedTimeOut = millis();
    while (Serial.available() > 0) {
        if (millis() - receivedTimeOut > 1000U) {
            receivedBufferIndex = 0;
            memset(receivedBuffer, 0, (ReceivedBufferLength + 1)); // Clear buffer
        }
        receivedTimeOut = millis();
        receivedChar = Serial.read();
        if (receivedChar == '\n' || receivedBufferIndex == ReceivedBufferLength) {
            receivedBufferIndex = 0;
            strupr(receivedBuffer);  // Convert to uppercase
            return true;
        } else {
            receivedBuffer[receivedBufferIndex] = receivedChar;
            receivedBufferIndex++;
        }
    }
    return false;
}

// Parse serial command
byte uartParse() {
    byte modeIndex = 0;
    if (strstr(receivedBuffer, "CALIBRATION") != NULL) modeIndex = 1;
    else if (strstr(receivedBuffer, "EXIT") != NULL) modeIndex = 4;
    else if (strstr(receivedBuffer, "ACID:") != NULL) modeIndex = 2;
    else if (strstr(receivedBuffer, "ALKALI:") != NULL) modeIndex = 3;
    return modeIndex;
}

// Perform pH calibration based on received command
void phCalibration(byte mode) {
    char *receivedBufferPtr;
    static bool acidCalibrationFinish = false, alkaliCalibrationFinish = false;
    static float acidValue, alkaliValue;
    static float acidVoltage, alkaliVoltage;
    float acidValueTemp, alkaliValueTemp, newSlopeValue, newInterceptValue;

    switch (mode) {
        case 0:
            // Command error if in calibration mode
            if (enterCalibrationFlag) Serial.println(F("Command Error"));
            break;

        case 1:
            // Enter calibration mode
            receivedBufferPtr = strstr(receivedBuffer, "CALIBRATION");
            enterCalibrationFlag = true;
            acidCalibrationFinish = false;
            alkaliCalibrationFinish = false;
            Serial.println(F("Enter Calibration Mode"));
            break;

        case 2:
            // Acid calibration
            if (enterCalibrationFlag) {
                receivedBufferPtr = strstr(receivedBuffer, "ACID:");
                receivedBufferPtr += strlen("ACID:");
                acidValueTemp = strtod(receivedBufferPtr, NULL);

                // Check if the acid value is within typical range
                if ((acidValueTemp > 3) && (acidValueTemp < 5)) { 
                    acidValue = acidValueTemp;
                    acidVoltage = averageVoltage / 1000.0;  // Convert mV to V
                    acidCalibrationFinish = true;
                    Serial.println(F("Acid Calibration Successful"));
                } else {
                    acidCalibrationFinish = false;
                    Serial.println(F("Acid Value Error"));
                }
            }
            break;

        case 3:
            // Alkali calibration
            if (enterCalibrationFlag) {
                receivedBufferPtr = strstr(receivedBuffer, "ALKALI:");
                receivedBufferPtr += strlen("ALKALI:");
                alkaliValueTemp = strtod(receivedBufferPtr, NULL);

                // Check if the alkali value is within typical range
                if ((alkaliValueTemp > 8) && (alkaliValueTemp < 11)) {
                    alkaliValue = alkaliValueTemp;
                    alkaliVoltage = averageVoltage / 1000.0;  // Convert mV to V
                    alkaliCalibrationFinish = true;
                    Serial.println(F("Alkali Calibration Successful"));
                } else {
                    alkaliCalibrationFinish = false;
                    Serial.println(F("Alkali Value Error"));
                }
            }
            break;

        case 4:
            // Finalize calibration if both acid and alkali calibration are completed
            if (enterCalibrationFlag) {
                if (acidCalibrationFinish && alkaliCalibrationFinish) {
                    newSlopeValue = (acidValue - alkaliValue) / (acidVoltage - alkaliVoltage);
                    EEPROM_write(SlopeValueAddress, newSlopeValue);

                    newInterceptValue = acidValue - (slopeValue * acidVoltage);
                    EEPROM_write(InterceptValueAddress, newInterceptValue);

                    Serial.println(F("Calibration Successful"));
                } else {
                    Serial.println(F("Calibration Failed"));
                }
                Serial.println(F("Exit Calibration Mode"));

                // Reset flags after calibration
                acidCalibrationFinish = false;
                alkaliCalibrationFinish = false;
                enterCalibrationFlag = false;
            }
            break;
    }
}


// Median filter to stabilize sensor readings
int getMedianNum(int bArray[], int iFilterLen) {
    int bTab[iFilterLen];

    // Copy input array to a temporary array for sorting
    for (int i = 0; i < iFilterLen; i++) {
        bTab[i] = bArray[i];
    }

    // Sort the array using Bubble Sort
    for (int j = 0; j < iFilterLen - 1; j++) {
        for (int i = 0; i < iFilterLen - j - 1; i++) {
            if (bTab[i] > bTab[i + 1]) {
                int bTemp = bTab[i];
                bTab[i] = bTab[i + 1];
                bTab[i + 1] = bTemp;
            }
        }
    }

    // Find and return the median
    int median;
    if (iFilterLen % 2 == 1) {
        median = bTab[iFilterLen / 2]; // Middle element for odd-length array
    } else {
        median = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2; // Average of two middle elements for even-length array
    }
    return median;
}


// Read slope and intercept from EEPROM, use defaults if uninitialized
void readCharacteristicValues() {
    // Read slope and intercept from EEPROM
    EEPROM_read(SlopeValueAddress, slopeValue);
    EEPROM_read(InterceptValueAddress, interceptValue);
    
    // Check if the EEPROM has uninitialized values (0xFF indicates erased EEPROM)
    if (EEPROM.read(SlopeValueAddress) == 0xFF && 
        EEPROM.read(SlopeValueAddress + 1) == 0xFF && 
        EEPROM.read(SlopeValueAddress + 2) == 0xFF && 
        EEPROM.read(SlopeValueAddress + 3) == 0xFF) {
        
        slopeValue = 3.5; // Default recommended slope if EEPROM is new
        EEPROM_write(SlopeValueAddress, slopeValue); // Store the default slope value in EEPROM
    }

    if (EEPROM.read(InterceptValueAddress) == 0xFF && 
        EEPROM.read(InterceptValueAddress + 1) == 0xFF && 
        EEPROM.read(InterceptValueAddress + 2) == 0xFF && 
        EEPROM.read(InterceptValueAddress + 3) == 0xFF) {
        
        interceptValue = 0.0; // Default recommended intercept if EEPROM is new
        EEPROM_write(InterceptValueAddress, interceptValue); // Store the default intercept value in EEPROM
    }
}


