# Convert Python AI model to binary code that can run on ESP32
import math
import numpy as np
from sklearn.svm import SVC
from tensorflow.lite.python.interpreter import Interpreter
from tensorflow.lite.python.lite import TFLiteKerasModelConverterV2
import tensorflow as tf
from tflite_model import get_model


if __name__ == "__main__":
    model: Interpreter = get_model()
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.optimizations = [tf.lite.Optimize.OPTIMIZE_FOR_SIZE]
    tflite_model = converter.convert()

    # Save the model to disk
    open("./output/model_opt.tflite", "wb").write(tflite_model)
