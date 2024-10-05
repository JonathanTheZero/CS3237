import os
from typing import Any, Literal
import paho.mqtt.client as mqtt
from time import sleep
import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.naive_bayes import GaussianNB
from sklearn.metrics import accuracy_score


CSV_FILE_PATH = "./training/weather.csv"
TRAIN_SIZE = 0.5
RANDOM_STATE = 42


WindowState = Literal["open", "closed", "partial"]


nb_classifier: GaussianNB | None = None


def train_model(
    csv_file_path: str = CSV_FILE_PATH,
    train_size: float = TRAIN_SIZE,
    random_state: int = RANDOM_STATE,
) -> None:
    global nb_classifier
    if not os.path.exists(csv_file_path):
        print(f"CSV file not found: {csv_file_path}")
        return

    try:
        nb_classifier = GaussianNB()

        data: pd.DataFrame = pd.read_csv(csv_file_path)
        X: pd.DataFrame = data[["temperature", "humidity"]]
        y: pd.Series[str] = data["window_state"]

        X_train, X_test, y_train, y_test = train_test_split(
            X, y, test_size=train_size, random_state=random_state
        )

        print("Training Naive Bayes model...")

        nb_classifier.fit(X_train, y_train)

        y_pred = nb_classifier.predict(X_test)
        accuracy = accuracy_score(y_test, y_pred)

        print(f"Naive Bayes Classifier Accuracy: {accuracy * 100:.2f}%")

        # print(
        #     nb_classifier.predict(
        #         pd.DataFrame([[29, 66]], columns=["temperature", "humidity"])
        #     )
        # )

    except Exception as e:
        print(f"Error during model training: {e}")


def predict(temperature: float, humidity: float) -> WindowState:
    global nb_classifier
    if nb_classifier is None:
        raise Exception("The model is not trained yet")

    result: np.ndarray = nb_classifier.predict(
        pd.DataFrame([[temperature, humidity]], columns=["temperature", "humidity"])
    )
    return result[0]
