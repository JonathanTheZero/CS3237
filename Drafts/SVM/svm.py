import numpy as np
import pandas as pd
from sklearn.svm import SVC
from sklearn.model_selection import train_test_split
import joblib
import os


MODEL_FILEPATH = "./svm_model.pk1"
DATA_FILEPATH = "./training_data.csv"


if __name__ == "__main__":
    data = pd.read_csv(DATA_FILEPATH)

    X = data.iloc[:, :-1].values
    y = data.iloc[:, -1].values
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.3, random_state=42
    )

    if os.path.exists(MODEL_FILEPATH):
        model: SVC = joblib.load(MODEL_FILEPATH)
        print("Model loaded from file.")
    else:
        model = SVC()
        model.fit(X_train, y_train)
        print("Model trained and saved to file.")
        joblib.dump(model, MODEL_FILEPATH)

    accuracy = model.score(X_test, y_test)
    print(f"Test set accuracy: {accuracy:.2f}")
