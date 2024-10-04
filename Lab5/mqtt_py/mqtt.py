from typing import Any
import paho.mqtt.client as mqtt
from time import sleep

SERVER_IP = "192.168.138.224"
SUBSCRIBE_TOPIC = "weather/#"
PUBLISH_TOPIC = "receiving/esp"


def on_connect(client: mqtt.Client, userdata: Any, flags, rc: int) -> None:
    print("Connected with result code: " + str(rc))
    client.subscribe(SUBSCRIBE_TOPIC)
    sleep(2)


def on_message(client: mqtt.Client, userdata: Any, message: mqtt.MQTTMessage) -> None:
    print(
        f"Received message on topic '{message.topic}' with content: '{message.payload.decode('UTF-8')}'"
    )

    if message.topic == "weather/temp":
        handle_temp(client, message)


def handle_temp(client: mqtt.Client, message: mqtt.MQTTMessage) -> None:
    temp_str = message.payload.decode("UTF-8")
    temp_value = float(temp_str.split(": ")[1].split(" ")[0])

    result_msg: tuple[str, int] = classify_message(temp_value)
    print("Sending back: ", result_msg[0])
    client.publish(PUBLISH_TOPIC, result_msg[0])


def classify_message(temperature: float) -> tuple[str, int]:
    classification: tuple[str, int] = "", -1
    if temperature > 30:
        classification = "Too hot.", 1
    elif temperature < 25:
        classification = "Too cold.", 2
    else:
        classification = "Moderate.", 0
    return classification


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("localhost", 1883, 60)
client.loop_forever()
