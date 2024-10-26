from time import sleep
import paho.mqtt.client as mqtt

SERVER_IP = "192.168.138.224"
TOPIC = "sound/track"


def on_connect(client: mqtt.Client, userdata, flags, rc: int) -> None:
    print("Connected with result code: " + str(rc))


client = mqtt.Client()
client.on_connect = on_connect
client.connect("localhost", 1883, 60)
client.loop_start()


try:
    while True:
        message: str = input("Type the track ID to send:")
        client.publish(TOPIC, message)
        print("Sent message:", message)

except KeyboardInterrupt:
    print("Disconnecting...")

finally:
    client.loop_stop()
    client.disconnect()
