from typing import Any
import paho.mqtt.client as mqtt
from time import sleep
import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.tree import DecisionTreeClassifier
import pickle

SERVER_IP = "192.168.138.224"
SUBSCRIBE_TOPIC = "weather/#"
PUBLISH_TOPIC = "receiving/esp"


def train_model() -> None:
    data = pd.read_csv("training/weather.csv")
    X = data[["humidity", "temperature"]]
    y = data["window_state"]

    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.2, random_state=42
    )

    model = DecisionTreeClassifier()
    model.fit(X_train, y_train)

    with open("window_classifier.pkl", "wb") as f:
        pickle.dump(model, f)


def load_model():
    with open("window_classifier.pkl", "rb") as f:
        model = pickle.load(f)
        return model


def classify_window(humidity, temperature, model):
    input_data = np.array([[humidity, temperature]])
    prediction = model.predict(input_data)
    return prediction[0]


def on_connect(client: mqtt.Client, userdata: Any, flags, rc: int) -> None:
    print("Connected with result code: " + str(rc))
    client.subscribe(SUBSCRIBE_TOPIC)


def on_message(client: mqtt.Client, userdata: Any, message: mqtt.MQTTMessage) -> None:
    print(
        f"Received message on topic '{message.topic}' with content: '{message.payload.decode('UTF-8')}'"
    )

    if message.topic == "weather/temp":
        handle_temp(client, message)


def handle_temp(client: mqtt.Client, message: mqtt.MQTTMessage) -> None:
    pass


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("localhost", 1883, 60)
client.loop_forever()
