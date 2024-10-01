import paho.mqtt.client as mqtt
from time import sleep


def on_connect(client: mqtt.Client, userdata, flags, rc: int) -> None:
    print("Connected with result code: " + str(rc))
    client.subscribe("hello/#")


def on_message(client: mqtt.Client, userdata, message: mqtt.MQTTMessage) -> None:
    print("Received message: '" + message.payload.decode("UTF-8") + "'")


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("localhost", 1883, 60)
client.loop_forever()
