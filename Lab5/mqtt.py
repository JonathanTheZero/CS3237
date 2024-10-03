from typing import Any
import paho.mqtt.client as mqtt
from time import sleep

SERVER_IP = "192.168.137.224"
TOPIC = "weather/#"


def on_connect(client: mqtt.Client, userdata: Any, flags, rc: int) -> None:
    print("Connected with result code: " + str(rc))
    client.subscribe(TOPIC)


def on_message(client: mqtt.Client, userdata: Any, message: mqtt.MQTTMessage) -> None:
    print(
        f"Received message on topic '{message.topic}' with content: '{message.payload.decode('UTF-8')}'"
    )


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("localhost", 1883, 60)
client.loop_forever()
