#!/usr/bin/env python
# import context # Ensures paho is in PYTHONPATH
import sqlite3

import paho.mqtt.client as mqtt
from dbSetup import *


# This happens when connecting
def on_connect(mqttc, obj, flags, rc):
    print("rc: " + str(rc))
    #setup subs
    #


# Getting a message from subscribe
def on_message(mqttc, obj, msg):
    print(msg.topic + " " + str(msg.qos) + " " + str(msg.payload))
    ##check the topic -> log in sensor or alarm table


# When something is published
def on_publish(mqttc, obj, mid):
    print("mid: " + str(mid))


# On subscribing to messages
def on_subscribe(mqttc, obj, mid, granted_qos):
    print("Subscribed: " + str(mid) + " " + str(granted_qos))


# Taking care of logging
def on_log(mqttc, obj, level, string):
    print(string)


# If you want to use a specific client id, use
# mqttc = mqtt.Client("client-id")
# but note that the client id must be unique on the broker. Leaving the client
# id parameter empty will generate a random id for you.
myhost = "mqtt.flespi.io"
token = "kic4ynNM9v8XyZowFAotKSgqLq7PBSOJYQDn1gZYBXMXLYkgowyWdCaOQ7shN4QC"

mqttc = mqtt.Client()
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_publish = on_publish
mqttc.on_subscribe = on_subscribe
# Uncomment to enable debug messages
# mqttc.on_log = on_log
mqttc.username_pw_set(token, token)
mqttc.connect(myhost, 1883)
mqttc.subscribe("shop/#", 0)
mqttc.loop_forever()
