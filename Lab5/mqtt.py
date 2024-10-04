from typing import Any
import paho.mqtt.client as mqtt
from time import sleep

SERVER_IP = "192.168.138.224"
TOPIC = "weather/#"


def on_connect(client: mqtt.Client, userdata: Any, flags, rc: int) -> None:
    print("Connected with result code: " + str(rc))
    client.subscribe(TOPIC)


def on_message(client: mqtt.Client, userdata: Any, message: mqtt.MQTTMessage) -> None:
    print(
        f"Received message on topic '{message.topic}' with content: '{message.payload.decode('UTF-8')}'"
    )

    if message.topic == "weather/temp":
        handle_temp(client, message)


def handle_temp(client: mqtt.Client, message: mqtt.MQTTMessage) -> None:
    temp_str = message.payload.decode("UTF-8")
    temp_value = float(temp_str.split(": ")[1].split(" ")[0])

    result_msg = classify_message(temp_value)
    print("Sending back: ", result_msg)
    client.publish("receiving/esp", result_msg)


def classify_message(temperature: float) -> str:
    if temperature > 30:
        classification = "Too hot, open the window."
    elif temperature < 25:
        classification = "Too cold, close the window."
    else:
        classification = "Temperature is moderate, partially open the window."
    return classification


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("localhost", 1883, 60)
client.loop_forever()
