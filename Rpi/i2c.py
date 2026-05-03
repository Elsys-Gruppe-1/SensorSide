from smbus2 import SMBus, i2c_msg
import requests
import time
import datetime
import json

# ESP32 adresser
SLAVE_ADDR_1 = 0x08
SLAVE_ADDR_2 = 0x09
SLAVE_ADDR_3 = 0x0A
WEB_URL = "https://stimle.elektra.io/api/upload"

# Liste for lokal lagring ved data ved når vi mister 4G (Buffer)
data_buffer = []

# Leser esp32 data ved bruk av adresse
def read_esp32_data(SLAVE_ADDR):
    try:
        with SMBus(1) as bus:
            msg = i2c_msg.read(SLAVE_ADDR, 128)
            bus.i2c_rdwr(msg)
            raw_bytes = list(msg)
            clean_bytes = bytearray([b for b in raw_bytes if 32 <= b <= 126])
            raw_data = clean_bytes.decode('utf-8').strip()
            
            if raw_data:
                start = raw_data.find('{')
                end = raw_data.rfind('}')
                if start != -1 and end != -1:
                    json_str = raw_data[start:end+1]
                    return json.loads(json_str)
            return None
    except Exception as e:
        print(f"I2C Tolkningsfeil esp({SLAVE_ADDR-7}): {e}")
        return None

def send_to_esp32(message, SLAVE_ADDR):
    try:
        with SMBus(1) as bus:
            data_to_send = [ord(c) for c in message]
            bus.write_i2c_block_data(SLAVE_ADDR, 0, data_to_send)
    except Exception as e:
        print(f"I2C Skrivefeil: {e}")

# Funksjon som sender JSON payload til web
def send_to_web(payload_list):
    try:
        all_success = True
        for item in payload_list:
            response = requests.post(url=WEB_URL, json=item, timeout=5)
            if response.status_code != 200:
                all_success = False
                break # Avbryt hvis en sending feiler
        return all_success
    except Exception as e:
        print(f"Nettverksfeil: {e}")
        return False

print("--- Raspberry Pi I2C Master m/Buffer Start ---")

try:
    while True:
        # Hent data fra alle tre sensor-noder
        sensor_data_1 = read_esp32_data(SLAVE_ADDR_1)
        sensor_data_2 = read_esp32_data(SLAVE_ADDR_2)
        sensor_data_3 = read_esp32_data(SLAVE_ADDR_3)
        
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        # Prosesser Node 1
        if sensor_data_1:
            sensor_data_1['ts'] = timestamp
            data_buffer.append(sensor_data_1)
            send_to_esp32("Pi mottok data!", SLAVE_ADDR_1)
            print(f"Node 1 lagt i buffer.")

        # Prosesser Node 2
        if sensor_data_2:
            sensor_data_2['ts'] = timestamp
            data_buffer.append(sensor_data_2)
            send_to_esp32("Pi mottok data!", SLAVE_ADDR_2)
            print(f"Node 2 lagt i buffer.")

        # Prosesser Node 3
        if sensor_data_3:
            sensor_data_3['ts'] = timestamp
            data_buffer.append(sensor_data_3)
            send_to_esp32("Pi mottok data!", SLAVE_ADDR_3)
            print(f"Node 3 lagt i buffer.")

        # Forsøk å sende hele bufferen hvis den inneholder data
        if data_buffer:
            print(f"Forsøker å sende buffer (størrelse: {len(data_buffer)})...")
            if send_to_web(data_buffer):
                # Tøm buffer hvis sendingen gikk
                data_buffer = []
            else:
                print("Buffer beholdes til neste forsøk.")

        print("-" * 30)
        time.sleep(10) # Måleintervall

except KeyboardInterrupt:
    print("\nAvslutter...")