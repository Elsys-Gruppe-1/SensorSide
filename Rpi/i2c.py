from smbus2 import SMBus, i2c_msg
import requests
import time
import datetime
import json

# ESP32 sin I2C-adresse (må matche koden på ESP32)
SLAVE_ADDR_1 = 0x08
SLAVE_ADDR_2 = 0x09
SLAVE_ADDR_3 = 0x0A
WEB_URL = "https://stimle.elektra.io/api/upload"

def read_esp32_data(SLAVE_ADDR):
    try:
        with SMBus(1) as bus:
            msg = i2c_msg.read(SLAVE_ADDR, 128)
            bus.i2c_rdwr(msg)
            
            # 1. Hent rådata som bytes
            raw_bytes = list(msg)
            
            # 2. Vask bort 0xFF og 0x00 (søppel fra I2C-bufferen)
            # Vi beholder bare bytes som er i det synlige ASCII-området
            clean_bytes = bytearray([b for b in raw_bytes if 32 <= b <= 126])
            
            # 3. Dekod til streng
            raw_data = clean_bytes.decode('utf-8').strip()
            
            if raw_data:
                # Finn starten og slutten på JSON-objektet for sikkerhets skyld
                start = raw_data.find('{')
                end = raw_data.rfind('}')
                if start != -1 and end != -1:
                    json_str = raw_data[start:end+1]
                    return json.loads(json_str)
            return None
            
    except Exception as e:
        print(f"I2C Tolkningsfeil esp({SLAVE_ADDR-7}):  {e}")
        return None

def send_to_esp32(message, SLAVE_ADDR):
    try:
        with SMBus(1) as bus:
            # Konverter streng til liste med bytes
            data_to_send = [ord(c) for c in message]
            bus.write_i2c_block_data(SLAVE_ADDR, 0, data_to_send)
            print(f"Sendte til ESP32: {message}")
    except Exception as e:
        print(f"I2C Skrivefeil: {e}")


# Funksjon som sender data til nettside
def send_to_web(data):
    try:
        response = requests.post(url = WEB_URL, json=data, timeout=5)
        if response.status_code == 200:
            print("data sent til nettside!!")
        else:
            print(f"Feilet sending til nettside med: {response.status_code}")
    except Exception as e:
        print(f"nettside error: {e}")

print("--- Raspberry Pi I2C Master Debug Start ---")
# post method eller post request
try:
    while True:
        # 1. Be om data fra ESP32
        sensor_data_1 = read_esp32_data(SLAVE_ADDR_1)
        sensor_data_2 = read_esp32_data(SLAVE_ADDR_2)
        sensor_data_3 = read_esp32_data(SLAVE_ADDR_3)
        
        if sensor_data_1:
            sensor_data_1['ts'] = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            print(f"Tolket JSON data1: {sensor_data_1}")
            temp = sensor_data_1.get("Values", {}).get("Temp")
            tds = sensor_data_1.get("Values", {}).get("TDS")
            # print(f"Temperatur: {temp}°C | TDS: {tds} ppm")
            
            # 2. Send et svar tilbake hvis vi fikk gyldig data
            send_to_esp32("Pi mottok data!", SLAVE_ADDR_1)

        if sensor_data_2:
            sensor_data_2['ts'] = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            print(f"Tolket JSON data2: {sensor_data_2}")
            temp = sensor_data_2.get("Values", {}).get("Temp")
            tds = sensor_data_2.get("Values", {}).get("TDS")
            # print(f"Temperatur: {temp}°C | TDS: {tds} ppm")
            
            # 2. Send et svar tilbake hvis vi fikk gyldig data
            send_to_esp32("Pi mottok data!", SLAVE_ADDR_2)

        if sensor_data_3:
            sensor_data_3['ts'] = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            print(f"Tolket JSON data3: {sensor_data_3}")
            temp = sensor_data_3.get("Values", {}).get("Temp")
            tds = sensor_data_3.get("Values", {}).get("TDS")
            # print(f"Temperatur: {temp}°C | TDS: {tds} ppm")
            
            # 2. Send et svar tilbake hvis vi fikk gyldig data
            send_to_esp32("Pi mottok data!", SLAVE_ADDR_3)
        
        if sensor_data_1:
            send_to_web(sensor_data_1)
        if sensor_data_2:
            send_to_web(sensor_data_2)
        if sensor_data_3:
            send_to_web(sensor_data_3)

        print("-" * 30)
        time.sleep(1)

except KeyboardInterrupt:
    print("\nAvslutter...") 