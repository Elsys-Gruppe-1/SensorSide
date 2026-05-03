import requests
import time

# ESP32 sin I2C-adresse (må matche koden på ESP32)
SLAVE_ADDR_1 = 0x08
SLAVE_ADDR_2 = 0x09
SLAVE_ADDR_3 = 0x0A
WEB_URL = "https://stimle.elektra.io/api/upload"


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

mock_package = {
    "ts":time.time(), # Timestamp; when data is sendt
    "pi_id":1, # id (int), id til raspberrypi-en som uplaodet daten
    "sensor_value":{ # Dict[Str] -> float | (Dict[ts] -> float), dataverdier til en sensor gitt som enten float eller list
        "Temperature": 4.12,
        "Salt": {time.time()-3: 12,
                 time.time()-2: 12.2,
                 time.time()-1: 12.4,
                 time.time(): 12.5}
    }
    }

if __name__ == "__main__":
    send_to_web(mock_package)