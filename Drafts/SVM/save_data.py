from typing import Any
import paho.mqtt.client as mqtt
import numpy as np


SERVER_IP = "192.168.138.224"
SUBSCRIBE_TOPIC = "esp/#"


def on_connect(client: mqtt.Client, userdata: Any, flags, rc: int) -> None:
    print("Connected with result code: " + str(rc))


def on_message(client: mqtt.Client, userdata: Any, message: mqtt.MQTTMessage) -> None:
    print(
        f"Received message on topic '{message.topic}' with content: '{message.payload.decode('UTF-8')}'"
    )
    if "fsrs" in message.topic:
        pass
    elif "load_cell" in message.topic:
        pass


if __name__ == "__main__":
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect("localhost", 1883, 60)
    client.loop_forever()
