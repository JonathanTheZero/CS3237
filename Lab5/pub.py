import paho.mqtt.client as mqtt
from time import sleep

SERVER_IP = "192.168.137.224"


def on_connect(client: mqtt.Client, userdata, flags, rc: int) -> None:
    print("Connected with result code: " + str(rc))
    print("Waiting for 2 seconds")
    sleep(2)

    print("Sending message.")
    client.publish("receiving/esp", "This is a test")


client = mqtt.Client()
client.on_connect = on_connect
client.connect(SERVER_IP, 1883, 60)
client.loop_forever()
