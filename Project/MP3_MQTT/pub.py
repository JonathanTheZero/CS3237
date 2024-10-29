from time import sleep
from typing import Any
import paho.mqtt.client as mqtt


SERVER_IP = "192.168.138.224"
PUBLISH_TOPIC = "esp32/1/track_id"
SUBSCRIBE_TOPIC = "esp32/#"


def on_connect(client: mqtt.Client, userdata, flags, rc: int) -> None:
    print("Connected with result code: " + str(rc))
    client.subscribe(SUBSCRIBE_TOPIC)


def on_message(client: mqtt.Client, userdata: Any, message: mqtt.MQTTMessage) -> None:
    print(
        f"Received message on topic '{message.topic}' with content: '{message.payload.decode('UTF-8')}'"
    )


client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message
client.connect("localhost", 1883, 60)
client.loop_start()


try:
    sleep(2)
    print()
    while True:
        message: str = input("Type the track ID to send:")
        client.publish(PUBLISH_TOPIC, message)
        print("Sent message to ESP32:", message)

except KeyboardInterrupt:
    print("\nDisconnecting...")

finally:
    client.loop_stop()
    client.disconnect()
