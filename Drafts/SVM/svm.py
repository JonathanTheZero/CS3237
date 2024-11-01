import numpy as np
from sklearn.svm import LinearSVC
from sklearn.pipeline import Pipeline
from sklearn.feature_extraction.text import TfidfTransformer
from sklearn.feature_extraction.text import CountVectorizer


import os
import numpy as np
from sklearn import svm
from sklearn.model_selection import train_test_split
import joblib


MODEL_FILEPATH = "./svm_model.pk1"
FEATURES_FILEPATH = "./iot_features.npy"
LABELS_FILEPATH = "iot_labels.npy"


if __name__ == "__main__":
    X = np.load(FEATURES_FILEPATH)
    y = np.load(LABELS_FILEPATH)

    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3)

    if os.path.exists(MODEL_FILEPATH):
        model: svm.SVC = joblib.load(MODEL_FILEPATH)
        print("Model loaded from file.")
    else:
        model = svm.SVC()
        model.fit(X_train, y_train)
        print("Model trained and saved to file.")
        joblib.dump(model, MODEL_FILEPATH)

    accuracy = model.score(X_test, y_test)
    print(f"Test set accuracy: {accuracy:.2f}")
