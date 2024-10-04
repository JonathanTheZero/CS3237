from typing import Any, Literal
import paho.mqtt.client as mqtt
from time import sleep
import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.naive_bayes import GaussianNB
from sklearn.metrics import accuracy_score


SERVER_IP = "192.168.138.224"
SUBSCRIBE_TOPIC = "weather/#"
PUBLISH_TOPIC = "receiving/esp"
CSV_FILE_PATH = "./training/weather.csv"


WindowState = Literal["open", "closed", "partial"]


nb_classifier = GaussianNB()


def train_model() -> None:
    data: pd.DataFrame = pd.read_csv(CSV_FILE_PATH)
    X: pd.DataFrame = data[["temperature", "humidity"]]
    y = data["window_state"]

    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.5, random_state=42
    )

    print("Training model...")
    nb_classifier.fit(X_train, y_train)

    y_pred = nb_classifier.predict(X_test)
    accuracy = accuracy_score(y_test, y_pred)

    print(f"Naive Bayes Classifier Accuracy: {accuracy * 100:.2f}%")

    # print(
    #     nb_classifier.predict(
    #         pd.DataFrame([[29, 66]], columns=["temperature", "humidity"])
    #     )
    # )


# Example: Assuming data format "Temp:26.5,Humidity:60.00"
def parse_sensor_data(data: str) -> tuple[float, float]:
    try:
        parts: list[str] = data.split(",")
        temp_str: str = parts[0].split(":")[1].strip()
        humidity_str: str = parts[1].split(":")[1].strip()
        return (float(temp_str), float(humidity_str))
    except:
        return (-1, -1)


def on_connect(client: mqtt.Client, userdata: Any, flags, rc: int) -> None:
    print("Connected with result code: " + str(rc))
    client.subscribe(SUBSCRIBE_TOPIC)
    train_model()
    sleep(1)


def on_message(client: mqtt.Client, userdata: Any, message: mqtt.MQTTMessage) -> None:
    print(
        f"Received message on topic '{message.topic}' with content: '{message.payload.decode('UTF-8')}'"
    )

    if message.topic.startswith("weather/"):
        handle_message_receive(client, message)


def handle_message_receive(client: mqtt.Client, message: mqtt.MQTTMessage) -> None:
    temperature, humidity = parse_sensor_data(message.payload.decode("UTF-8"))

    if temperature == -1 or humidity == -1:
        print("Something went wrong when trying to parse the data")
        return

    prediction: WindowState = nb_classifier.predict(
        pd.DataFrame([[temperature, humidity]], columns=["temperature", "humidity"])
    )[0]

    print("Predicted and sending back: ", prediction)
    client.publish(PUBLISH_TOPIC, prediction)


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("localhost", 1883, 60)
client.loop_forever()
