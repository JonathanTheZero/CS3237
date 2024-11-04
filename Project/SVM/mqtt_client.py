from typing import Any, Literal
import paho.mqtt.client as mqtt
from time import sleep
import numpy as np
from svm import load_and_train_model, predict_seat_position


SEAT_POSITIONS_TRACK_MAPPING: dict[str, int] = {
    "no_contact": -1,
    "beep": 1,
    "slouching": 4,
    "backward_lean": 2,
    "leaning_back": 2,
    "imbalance_left": 5,
    "imbalance_right": 5,
    "eyes_too_far": 6,
    "eyes_too_close": 3,
}
SERVER_IP = "192.168.138.224"
SUBSCRIBE_TOPIC = "esp/#"
PUBLISH_TOPIC = "esp/2/track_id"


# Expects the following format: 1,2,3,4,5,6,7,8
def parse_sensor_data(data: str) -> list[int]:
    try:
        array: list[int] = [int(num) for num in data.split(",")]
        return array
    except Exception as e:
        print(f"Error parsing data string: {data}, failed with exception {e}")
        return []


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
        handle_data_receival(client, message)


def handle_data_receival(client: mqtt.Client, message: mqtt.MQTTMessage) -> None:
    global SEAT_POSITIONS_TRACK_MAPPING
    data: list[int] = parse_sensor_data(message.payload.decode("UTF-8"))
    if data == []:
        return

    prediction: str = predict_seat_position(data)
    message_code: int = SEAT_POSITIONS_TRACK_MAPPING.get(prediction, -1)

    if message_code == -1:
        if prediction != "no_contact":
            print("Error parsing SVM response:", prediction)
        return

    print("Predicted and sending back: ", message_code)
    client.publish(PUBLISH_TOPIC, message_code)


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("localhost", 1883, 60)
client.loop_forever()
