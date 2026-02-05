#include <OneWire.h>

int DS18S20_Pin = 3; // Pin connected to the DS18S20 sensor signal
OneWire ds(DS18S20_Pin); // OneWire bus setup on pin 2

void setup(void) {
  Serial.begin(115200); // Initialize serial communication at 9600 bps
  pinMode(3, INPUT_PULLUP);
}

void loop(void) {
  float temperature = getTemp(); // Get the temperature from the sensor
  Serial.println(temperature);   // Print the temperature to the serial monitor
  delay(100); // Delay for readability (100 ms)
}

float getTemp() {
  // This function reads temperature from one DS18S20 sensor in Celsius

  byte data[12];   // Array to hold sensor data
  byte addr[8];    // Array to hold sensor address
  
  // Search for a sensor on the bus
  if (!ds.search(addr)) {
    ds.reset_search(); // No more sensors, reset search
    return -1000;      // Return error value if no sensor is found
  }
  
  // Check CRC to validate sensor data
  if (OneWire::crc8(addr, 7) != addr[7]) {
    Serial.println("CRC is not valid!");
    return -1000; // Return error if CRC fails
  }
  
  // Check that the device is a DS18S20 or compatible
  if (addr[0] != 0x10 && addr[0] != 0x28) {
    Serial.println("Device is not recognized");
    return -1000; // Return error if the device is not recognized
  }

  ds.reset();        // Reset the OneWire bus
  ds.select(addr);   // Select the sensor using its address
  ds.write(0x44, 1); // Start temperature conversion (parasite power enabled)
  
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);    // Read the scratchpad (sensor data)
  
  // Read 9 bytes of data from the sensor
  for (int i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
  
  ds.reset_search(); // Reset search for the next loop
  
  // Combine MSB and LSB into a single temperature value
  byte MSB = data[1]; // Most Significant Byte
  byte LSB = data[0]; // Least Significant Byte
  float tempRead = ((MSB << 8) | LSB); // Convert to 16-bit integer
  float TemperatureSum = tempRead / 16.0; // Convert to Celsius

  return TemperatureSum; // Return temperature in Celsius
}
