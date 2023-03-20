#!/usr/bin/env python
# import context # Ensures paho is in PYTHONPATH
from datetime import datetime, timedelta
import time
import paho.mqtt.client as mqtt
from dbActions import *
import queue

waitingForResponse = False


# This happens when connecting
def on_connect(mqttc, obj, flags, rc):
    print("rc: " + str(rc))
    # setup subs
    #


# Getting a message from subscribe
def on_message(mqttc, obj, msg):
    print(msg.topic + " " + str(msg.qos) + " " + str(msg.payload))
    ##check the topic -> log in sensor or alarm table
    if "bedStatus/" in str(msg.topic):
        logSensorData(str(msg.topic), str(msg.payload.decode("utf-8")))
    if "alert/" in str(msg.topic):
        resident_id = str(msg.topic).split("/")[1]
        logAlertData(resident_id)
        residentInfo = getResidentInfo(resident_id)
        print("res1: " + residentInfo)
        alertQueue.put(residentInfo)
    if "adminResponse" in str(msg.topic):
        global waitingForResponse
        waitingForResponse = False
        alertResponse = str(msg.payload.decode("utf-8"))
        responseArr = alertResponse.split(",")
        updateAlert(responseArr[1], responseArr[0])




# If you want to use a specific client id, use
# mqttc = mqtt.Client("client-id")
# but note that the client id must be unique on the broker. Leaving the client
# id parameter empty will generate a random id for you.
myhost = "mqtt.flespi.io"
token = "kic4ynNM9v8XyZowFAotKSgqLq7PBSOJYQDn1gZYBXMXLYkgowyWdCaOQ7shN4QC"

mqttc = mqtt.Client()
mqttc.on_message = on_message
mqttc.on_connect = on_connect
# Uncomment to enable debug messages
# mqttc.on_log = on_log
mqttc.username_pw_set(token, token)
mqttc.connect(myhost, 1883)
mqttc.subscribe("bedStatus/#")
mqttc.subscribe("alert/#")
mqttc.subscribe("adminResponse")
mqttc.loop_start()


def sendWakeup(current_time, minutesLater):
    list = checkAwake(current_time, minutesLater)
    for id in list:
        print(id)
        mqttc.publish("bedtime/" + str(id), "deactivate", 0)


def sendBedtime(current_time, minutesLater):
    list = checkBedtime(current_time, minutesLater)
    for id in list:
        print(id)
        mqttc.publish("bedtime/" + str(id), "activate", 0)


alertQueue = queue.Queue()

goAgain = True
minutes = 1
start_time_min = datetime.now().minute
while goAgain:
    # wakeup calls and bedtimes
    current_time = datetime.now()
    if current_time.minute - start_time_min >= minutes:  # 0 for testing :)
        start_time_min = datetime.now().minute
        minutesLater = current_time + timedelta(minutes=minutes)

        current_formatted = datetime.strftime(current_time, "%H:%M:%S")
        later_formatted = datetime.strftime(minutesLater, "%H:%M:%S")
        sendWakeup(current_formatted, later_formatted)
        sendBedtime(current_formatted, later_formatted)

    # Alert Queue
    if alertQueue.qsize() > 0 and waitingForResponse == False:  # why can I not just !waitingforresponse
        waitingForResponse = True
        resident = alertQueue.get()
        print("res2: " + resident)
        msgString = resident.replace(" ", "")
        print(msgString)
        mqttc.publish("adminAlert", msgString, 0)

    time.sleep(1)  # tiny delay
