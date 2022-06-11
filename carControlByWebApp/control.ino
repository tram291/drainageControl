#include <EspMQTTClient.h>
#include <ESP32Servo.h>
#include <analogWrite.h>
#include <WiFi.h>

#define SERVO_PIN 13
#define AIN1 12 // day tren
#define AIN2 14 // day duoi
#define PWMA 27
#define BIN1 26  // day tren
#define BIN2 25 // day duoi
#define PWMB 33

const char* ssid = "TP-LINK_4E748E";
const char* password = "01242065";
const char* ssid1 = "UIT Public";
const char* password1 = "";
const char* mqtt_server = "mqtt.flespi.io";
const char* mqtt_user = "jA6b1JD1PaXIlOsvtzQ5JwxHRMmgPtmZRgYGPqxuqi3zWjTgHzbgVyLB9VHOeTBT";
const char* clientID = "mqtt-board-panel-24183321";
String state="";
String control="";
Servo servo;
int angle = 0;
int SPEED;

EspMQTTClient _client(mqtt_server, 
                      1883,
                      mqtt_user,
                      "",
                      clientID);

void onConnectionEstablished() {
  _client.subscribe("control", [] (const String &payload)  {
    Serial.println(payload);
    control = payload;
  });
  _client.subscribe("state", [] (const String &payload)  {
    Serial.println(payload);
    state = payload;
  });
  _client.subscribe("speed", [] (const String &payload)  {
    Serial.println(payload);
    SPEED = payload.toInt();
  });
}

void initWifi() {
  int count;
  WiFi.begin(ssid, password);
  for (count = 0; WiFi.status() != WL_CONNECTED && count < 15; count ++) {
    Serial.print('.');
    delay(500);
    
  }
  if (count >= 15) {
    WiFi.begin(ssid1, password1);
    for (count = 0; WiFi.status() != WL_CONNECTED && count < 10; count ++) {
      Serial.print('.');
      delay(500);
    }
  }
  Serial.println("Wifi connected");
}

void initServo() {
  servo.setPeriodHertz(50);  
  servo.attach(SERVO_PIN, 500, 2400);
}


void setup() {
  Serial.begin(115200);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

  initWifi();
  initServo();
}
//===================================end setup===================
void loop() {
  _client.loop();
  if(state == "go") {
    switch(control.charAt(0)) {
      case 'f': 
        forward(150);
        break;
      case 'b': 
        backward(150);
        break;
      case 'l': 
        turnLeft(175);
        break;
      case 'r': 
        turnRight(110);
        break;
    }
  }
  else {
    stop_motor();
    angle = 150;
  }
}

//===================================end loop===================
void forward(int angle)
{
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  analogWrite(PWMB, SPEED);
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  analogWrite(PWMA, SPEED);
  servo.write(angle);
  Serial.println("forward");Serial.println(angle); 
}
void backward(int angle)
{
  digitalWrite(AIN1, LOW);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, SPEED);
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, SPEED);
  servo.write(angle);Serial.println(angle);

  Serial.println("backward");
}
void turnLeft(int angle)
{
  servo.write(angle);Serial.println(angle);
  analogWrite(PWMB, SPEED/2);
  analogWrite(PWMA, SPEED);
  Serial.println("turning left");
}

void turnRight(int angle)
{
  servo.write(angle); Serial.println(angle);
  analogWrite(PWMB, SPEED);
  analogWrite(PWMA, SPEED/2);
  Serial.println("turning right");
}

void stop_motor()
{
  digitalWrite(BIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(AIN1, LOW);

  for (; SPEED > 0; SPEED -=20)
  {
    analogWrite(PWMB, SPEED);
    analogWrite(PWMA, SPEED);
    delay(200);
  }
  servo.write(angle); Serial.println(angle);
  Serial.println("stop motor");
}
