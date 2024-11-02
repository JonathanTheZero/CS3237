from typing import Any, Literal
import paho.mqtt.client as mqtt
from time import sleep
import numpy as np
from svm import load_and_train_model, predict_seat_position


SERVER_IP = "192.168.138.224"
SUBSCRIBE_TOPIC = "esp/#"
PUBLISH_TOPIC = "receiving/esp"


# Expects the following format: 1,2,3,4,5,6,7,8
def parse_sensor_data(data: str) -> list[int]:
    try:
        array = [int(num) for num in data.split(",")]
        return array
    except Exception as e:
        print(f"Error parsing data: {e}")
        raise e


def on_connect(client: mqtt.Client, userdata: Any, flags, rc: int) -> None:
    print("Connected with result code: " + str(rc))
    client.subscribe(SUBSCRIBE_TOPIC)
    load_and_train_model()
    sleep(1)
    # print(predict_seat_position([0, 0, 0, 0]))
    # print(predict_seat_position([0, 834, 244, 0]))
    # print(predict_seat_position([234, 0, 0, 5675]))
    # print(predict_seat_position([834, 900, 900, 900]))


def on_message(client: mqtt.Client, userdata: Any, message: mqtt.MQTTMessage) -> None:
    print(
        f"Received message on topic '{message.topic}' with content: '{message.payload.decode('UTF-8')}'"
    )

    if message.topic == "esp/data":
        handle_message_receive(client, message)


def handle_message_receive(client: mqtt.Client, message: mqtt.MQTTMessage) -> None:
    data = parse_sensor_data(message.payload.decode("UTF-8"))
    prediction = predict_seat_position(data)

    print("Predicted and sending back: ", prediction)
    client.publish(PUBLISH_TOPIC, prediction)


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("localhost", 1883, 60)
client.loop_forever()
