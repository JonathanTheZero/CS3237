from typing import Any
import paho.mqtt.client as mqtt
import numpy as np


SERVER_IP = "192.168.138.224"
SUBSCRIBE_TOPIC = "esp/#"
ESP_IS_ONLINE: list[bool] = [False, False, False]
FSR_DATA: dict[int, list[int]] = {}
LOAD_CELL_DATA: dict[int, list[int]] = {}


def on_connect(client: mqtt.Client, userdata: Any, flags, rc: int) -> None:
    print("Connected with result code: " + str(rc))


def on_message(client: mqtt.Client, userdata: Any, message: mqtt.MQTTMessage) -> None:
    print(
        f"Received message on topic '{message.topic}' with content: '{message.payload.decode('UTF-8')}'"
    )

    if "startup" in message.topic:
        sync_devices(client, message)
    elif "fsrs" in message.topic:
        handle_iot_data(message, "fsr")
    elif "load_cell" in message.topic:
        handle_iot_data(message, "load_cell")
    elif "save" in message.topic:
        save_data()


def sync_devices(client: mqtt.Client, message: mqtt.MQTTMessage) -> None:
    global ESP_IS_ONLINE
    id: int = int(message.topic.split("/")[1])
    ESP_IS_ONLINE[id] = True
    if not False in ESP_IS_ONLINE:
        for i in range(len(ESP_IS_ONLINE)):
            client.publish(f"esp/{i}/id", 0)


# Expected format for thsoe id: val1,val2,val3,val4
def handle_iot_data(message: mqtt.MQTTMessage, sensor_type: str) -> None:
    global FSR_DATA, LOAD_CELL_DATA
    message_str: str = message.payload.decode("UTF-8")
    id_str, vals_str = message_str.split(": ")
    id: int = int(id_str)
    vals: list[int] = [int(value) for value in vals_str.split(",")]
    if sensor_type == "fsr":
        FSR_DATA[id] = vals
    elif sensor_type == "load_cell":
        LOAD_CELL_DATA[id] = vals
    else:
        print("Unknown sensor type")


def save_data() -> np.ndarray:
    global FSR_DATA, LOAD_CELL_DATA
    unified_data = []
    for i in range(min(len(FSR_DATA), len(LOAD_CELL_DATA))):
        unified_data.append(FSR_DATA[i] + LOAD_CELL_DATA[i])

    return np.array(unified_data)


if __name__ == "__main__":
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect("localhost", 1883, 60)
    client.loop_forever()
