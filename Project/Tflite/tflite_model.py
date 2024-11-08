import os

# Do not use Cuda cores as my laptop doesn't have any
os.environ["CUDA_VISIBLE_DEVICES"] = "-1"
# Disable extensive logging during compiling
os.environ["TF_CPP_MIN_LOG_LEVEL"] = "3"

from typing import Any, Optional
import numpy as np
import tensorflow as tf
from keras import Sequential, layers
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder
import pandas as pd
from tensorflow.lite.python.lite import TFLiteKerasModelConverterV2
from OrderedEncoder import OrderedLabelEncoder
from numpy._typing._array_like import NDArray


TFLITE_MODEL: Optional[tf.lite.Interpreter] = None
LABEL_ENCODER_FILEPATH = "./output/label_encoder.npy"
CSV_FILEPATH = "./training_data.csv"
MODEL_FILEPATH = "./output/model.tflite"
POSITIONS_ORDERED: list[str] = [
    "no_contact",
    "perfect_posture",
    "leaning_back",
    "slouching",
    "leaning_forward",
    "leaning_left",
    "leaning_right",
]


def train_and_load_model() -> None:
    global TFLITE_MODEL, POSITIONS_ORDERED

    data: pd.DataFrame = pd.read_csv(CSV_FILEPATH)
    X = data.iloc[:, :-1].values.astype(float)
    y = data.iloc[:, -1].values

    label_encoder = OrderedLabelEncoder()
    label_encoder.fit(POSITIONS_ORDERED)

    y = label_encoder.transform(y)  # type: ignore
    np.save(LABEL_ENCODER_FILEPATH, label_encoder.classes_)

    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.2, random_state=42
    )

    model = Sequential(
        [
            layers.Input(shape=(8,)),
            layers.Dense(64, activation="relu"),
            layers.Dense(32, activation="relu"),
            layers.Dense(16, activation="relu"),
            layers.Dense(len(np.unique(y)), activation="softmax"),
        ]
    )

    model.compile(
        optimizer="adam", loss="sparse_categorical_crossentropy", metrics=["accuracy"]
    )
    model.fit(
        X_train, y_train, epochs=50, batch_size=4, validation_data=(X_test, y_test)
    )

    loss, accuracy = model.evaluate(X_test, y_test)
    print(f"Test Accuracy: {accuracy:.4f}")

    converter: TFLiteKerasModelConverterV2 = tf.lite.TFLiteConverter.from_keras_model(
        model
    )
    converter.optimizations = {tf.lite.Optimize.DEFAULT}
    model_bytes = converter.convert()
    with open(MODEL_FILEPATH, "wb") as f:
        f.write(model_bytes)  # type: ignore
    TFLITE_MODEL = tf.lite.Interpreter(model_path=MODEL_FILEPATH)
    TFLITE_MODEL.allocate_tensors()


def get_model():
    global TFLITE_MODEL
    if TFLITE_MODEL is None:
        raise Exception("The model has not been trained yet.")
    return TFLITE_MODEL


def load_model() -> None:
    global TFLITE_MODEL
    if os.path.exists(MODEL_FILEPATH):
        TFLITE_MODEL = tf.lite.Interpreter(model_path=MODEL_FILEPATH)
        TFLITE_MODEL.allocate_tensors()
        print("TFLite model loaded from file.")
    else:
        print("TFLite model not found, training a new model.")
        train_and_load_model()


def predict_seat_position(data: list[int]):
    global TFLITE_MODEL
    if TFLITE_MODEL is None:
        raise Exception("The model is not trained yet.")

    input_details = TFLITE_MODEL.get_input_details()
    output_details = TFLITE_MODEL.get_output_details()

    input_data: np.ndarray[Any, np.dtype[np.float32]] = np.array(
        [data], dtype=np.float32
    )
    TFLITE_MODEL.set_tensor(input_details[0]["index"], input_data)
    TFLITE_MODEL.invoke()
    prediction = np.argmax(TFLITE_MODEL.get_tensor(output_details[0]["index"]), axis=-1)

    label_encoder_classes = np.load(LABEL_ENCODER_FILEPATH, allow_pickle=True)
    return label_encoder_classes[prediction[0]], prediction[0]


load_model()

print(predict_seat_position([0, 0, 0, 0, 0, 0, 0, 0]))
print(predict_seat_position([15000, 16002, 19186, 16092, 0, 834, 244, 0]))
print(predict_seat_position([10000, 10000, 15000, 19000, 234, 0, 0, 5675]))
print(predict_seat_position([18300, 16200, 19999, 12000, 834, 900, 900, 900]))
print(predict_seat_position([19047, 17051, 18310, 16747, 0, 1489, 870, 0]))
