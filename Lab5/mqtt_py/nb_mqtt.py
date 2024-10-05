from typing import Any, Literal
import paho.mqtt.client as mqtt
from time import sleep
from nb_classifier import WindowState, train_model, predict_window_state


SERVER_IP = "192.168.138.224"
SUBSCRIBE_TOPIC = "weather/#"
PUBLISH_TOPIC = "receiving/esp"


# Expects the following format:
# Temperature XX.XX °C, Humidity: XX.XX%
def parse_sensor_data(data: str) -> tuple[float, float]:
    try:
        parts: list[str] = data.split(", ")
        temp_str: str = parts[0].replace("Temperature: ", "").replace("°C", "").strip()
        humidity_str: str = parts[1].replace("Humidity: ", "").replace("%", "").strip()
        return (float(temp_str), float(humidity_str))
    except Exception as e:
        print(f"Error parsing data: {e}")
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

    if message.topic == "weather/data":
        handle_message_receive(client, message)


def handle_message_receive(client: mqtt.Client, message: mqtt.MQTTMessage) -> None:
    temperature, humidity = parse_sensor_data(message.payload.decode("UTF-8"))

    if temperature == -1 or humidity == -1:
        print("Invalid sensor data. Not sending a response.")
        return

    prediction: WindowState = predict_window_state(temperature, humidity)

    print("Predicted and sending back: ", prediction)
    client.publish(PUBLISH_TOPIC, prediction)


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("localhost", 1883, 60)
client.loop_forever()
