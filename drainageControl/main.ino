
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>

#define AUTH "AUTH_TOKEN"
#define HOST "HOST_URL"
#define WATER_LEVEL A0
#define DROPRAIN D0
#define LIMIT_HIGH_LEVEL 475
#define LIMIT_LOW_LEVEL 250
#define CONTROL_PIN D7


char* ssid = "*******";
char* password = "*";
String isPump = "";
String node = "node1/";
bool isRain = "";
bool isPumping = false;
int level;
int second;
FirebaseData data;

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(WATER_LEVEL, INPUT);
  pinMode(DROPRAIN, INPUT);
  pinMode(CONTROL_PIN, OUTPUT);

  //connect to wifi
  Serial.println("connecting to wifi");
  WiFi.begin(ssid, password);
  Serial.println("started connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("wifi connected");

  //connect to firebase
  Firebase.begin(HOST, AUTH);
  Firebase.reconnectWiFi(true);
  data.setBSSLBufferSize(1024, 1024);
  data.setResponseSize(1024);
  Serial.println("end setup");
}
//===========================end setup==================================
void pump();
void stop_pumping ();
void read_fb();
bool send_water_level(int);
bool send_rain_sensor_value (bool);
void auto_control(int, bool);

void loop() {
  isRain = !digitalRead(DROPRAIN);
  level = analogRead(WATER_LEVEL);
  if (WiFi.status() == WL_CONNECTED) {
    read_fb();
    if (second >= 5) {
      auto_control(480, isRain);
      Serial.printf(" auto second: %d\n", second);
    }
//    send_water_level(480);
    send_rain_sensor_value(isRain);
    second++;
    Serial.printf("second: %d\n", second);
    delay(1000);
  }
  else {
    auto_control(level, isRain);
  }
}
//============================end loop===================================
void read_fb() {
  //read pump request from node 1
  String path = node + "BOM";
  if(!Firebase.getString(data, path)) {
    Serial.println(data.errorReason());
    return;
  }
  if(!Firebase.getString(data, "/node1/level")) {
    Serial.println(data.errorReason());
    return;
  }
  level = data.to<int>();
  isPump = data.to<String>();
  isPump.toLowerCase();
  if (isPump == "true") {
    if (!isPumping) {
      second = 0;
      pump();
    }
  }
  else {
    if (isPumping) {
     second = 0;
     stop_pumping(); 
    }
  }
}

void pump(){
  Serial.println("pumping");
  isPumping = true;
  digitalWrite(CONTROL_PIN, HIGH);
}

void stop_pumping(){
  Serial.println("stop pumping");
  isPumping = false;
  digitalWrite(CONTROL_PIN, LOW);
}

bool send_water_level(int level) {
  Serial.println(level);
  if(Firebase.setString(data, node + "WATER_LEVEL", String(level))) {
    data.errorReason();
    return false;
  }
  Serial.println("sent water level");
  return true;
}

bool send_rain_sensor_value (bool s) {
  String isRain = s ? "TRUE" : "FALSE";
  if(Firebase.setString(data, node + "SENSOR_MUA", isRain)) {
    data.errorReason();
    return 0;
  }
  Serial.println("sent rain status");
  return 1;
}

void auto_control(int level, bool isRain) {
  if(isRain && level > LIMIT_HIGH_LEVEL) {
    pump();
  }
  else {
    if(level <= LIMIT_LOW_LEVEL)
      stop_pumping();
  }
  
  String _pump = isPumping ? "TRUE" : "FALSE";
  if(Firebase.setString(data, node + "BOM", _pump)) {
    data.errorReason();
  }
}
