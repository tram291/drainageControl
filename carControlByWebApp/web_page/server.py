import re
from flask import Flask, render_template, request
from flask_mqtt import Mqtt
from flask_mqtt import MQTT_LOG_ERR
from flask_cors import CORS

app = Flask(__name__)
cors = CORS(app)

app.config['MQTT_BROKER_URL'] = 'mqtt.flespi.io'
app.config['MQTT_USERNAME'] = 'jA6b1JD1PaXIlOsvtzQ5JwxHRMmgPtmZRgYGPqxuqi3zWjTgHzbgVyLB9VHOeTBT'
app.config['MQTT_REFRESH'] = 1.0
app.config['MQTT_BROKER_PORT'] = 1883
mqtt = Mqtt(app)

data = [""]
@mqtt.on_connect()
def handle_mqtt(client, userdata, flags, rc):
    print("MQTT BROKER CONNECTED")
    mqtt.subscribe('control')
    mqtt.subscribe('state')
    mqtt.subscribe('speed')
    mqtt.subscribe('camera_url')

@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    print('recive message')
    print(message.topic)
    if message.topic == "camera_url":
        data.append(message.payload.decode())


    

@app.route("/", methods=['GET', 'POST'])
def index():
    print(request.method)
    result = ""
    icon = "stop"
    speed = 100 
    print(data[-1])
    if request.method == "POST":
        result = request.form.get('current_control_name')
        icon = request.form.get('state')
        speed = request.form.get('speed')
        mqtt.publish('control', result)
        mqtt.publish('state', icon)
        mqtt.publish('speed', speed)
    return render_template("index1.html", 
                            state_arg = icon, 
                            c_arg=result, 
                            url_arg = data[-1],
                            speed_arg = speed)
if __name__ == '__main__':
    app.run()
