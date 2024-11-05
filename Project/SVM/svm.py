from typing import Literal, Optional
import numpy as np
import pandas as pd
from sklearn.svm import SVC
from sklearn.model_selection import train_test_split
import joblib
import os


MODEL_FILEPATH = "./svm_model.pk1"
DATA_FILEPATH = "./training_data.csv"
SVM_MODEL: Optional[SVC] = None


# If the model already exists as a file, load it from there
# Otherwise train it using the sample data
def load_and_train_model():
    global SVM_MODEL
    data: pd.DataFrame = pd.read_csv(DATA_FILEPATH)
    X: np.ndarray = data.iloc[:, :-1].values
    y = data.iloc[:, -1].values
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.3, random_state=42
    )

    if os.path.exists(MODEL_FILEPATH):
        SVM_MODEL = joblib.load(MODEL_FILEPATH)
        print("Model loaded from file.")
    else:
        SVM_MODEL = SVC(random_state=42)
        SVM_MODEL.fit(X_train, y_train)
        print("Model trained and saved to file.")
        joblib.dump(SVM_MODEL, MODEL_FILEPATH)

    if SVM_MODEL is None:
        raise Exception("Something went wrong during loading or training the model.")

    accuracy = SVM_MODEL.score(X_test, y_test)
    print(f"Test set accuracy: {accuracy:.2f}")


def predict_seat_position(data: list[int]) -> str:
    global SVM_MODEL
    if SVM_MODEL is None:
        raise Exception("The model is not trained yet.")

    result: np.ndarray = SVM_MODEL.predict(pd.DataFrame([data]))

    return str(result[0])
