#include "BLEDevice.h"
#include <analogWrite.h>
#define AIN1 13 // day tren
#define AIN2 12 // day duoi
#define PWMA 15 
#define BIN1 4  // day tren
#define BIN2 14 // day duoi
#define PWMB 2
#define LED 3
bool FRONT, LEFT, RIGHT;
int SPEED;

static BLEUUID serviceUUID("ab8ae9b7-1881-486b-9452-0a7f6160e774");
static BLEUUID    charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");
    pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
  
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks



void setup() {
  Serial.begin(115200);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(LED, OUTPUT);

  digitalWrite(LED, LOW);
  SPEED = 0;
  
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
  Serial.println("end setup");
} // End of setup.

//----------------------loop---------------
void forward();
void backward();
void turnLeft();
void turnRight();
void stop_motor();
const char* v;
const char* front = "forward";
const char* left = "left";
const char* right = "right";
const char* off = "off";
const char* up = "up";
const char* down = "down";
const char* back = "backward";

bool is_off = false;

void loop() {
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }
  
  if (connected) 
  {        
    if(pRemoteCharacteristic->canRead())
    {
      v = pRemoteCharacteristic->readValue().c_str();
      Serial.print("data's sent by server is ");
      Serial.println(v);
      
      if (strcmp(v, front) == 0)
        forward(); 
      if (strcmp(v, back) == 0)
        backward();      
      if (strcmp(v, left) == 0)
        turnLeft();
      if (strcmp(v, right) == 0)
        turnRight();
      if (strcmp(v, off) == 0)
        stop_motor();
      if (strcmp(v, up) == 0 && SPEED <= 240)
      {
        SPEED += 20;
        delay(300);
      }
      if (strcmp(v, down) == 0 && SPEED >= 10)
      {
        SPEED -= 20;
        delay(300);
      }
      Serial.println(SPEED);
    }
  }
  else if(doScan){
    BLEDevice::getScan()->start(3);  
    Serial.println("no device found");
  }

}

//-----------------------cac ham dieu huong theo tin hieu ble-----------------------
void forward()
{
  if (is_off) // dang o trang thai "off" -> chuyen sang trang thai "on"
  {
    is_off = false;
    SPEED = 100;
  }
  digitalWrite(LED, HIGH);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  analogWrite(PWMB, SPEED);
  
  digitalWrite(AIN2, HIGH);
  digitalWrite(AIN1, LOW);
  analogWrite(PWMA, SPEED);

  while (strcmp(v, front) == 0)
  {
    analogWrite(PWMB, SPEED);
    analogWrite(PWMA, SPEED);
    v = pRemoteCharacteristic->readValue().c_str();
    Serial.println("going forward");
    Serial.println(SPEED);
  }
}
void backward()
{
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, SPEED);
  
  digitalWrite(AIN2, LOW);
  digitalWrite(AIN1, HIGH);
  analogWrite(PWMA, SPEED);

  while (strcmp(v, back) == 0)
  {
    analogWrite(PWMB, SPEED);
    analogWrite(PWMA, SPEED);
    v = pRemoteCharacteristic->readValue().c_str();
    Serial.println("going backward");
    Serial.println(SPEED);
  }
}
void turnLeft()
{
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, 0);
  
  digitalWrite(AIN2, HIGH);
  digitalWrite(AIN1, LOW);
  analogWrite(PWMA, SPEED);
  
  while (strcmp(v, left) == 0)
  {
    analogWrite(PWMA, SPEED);
    v = pRemoteCharacteristic->readValue().c_str();
    Serial.println("going to the left");
    Serial.println(SPEED);
  }
}

void turnRight()
{
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, 0);
  
  digitalWrite(BIN2, HIGH);
  digitalWrite(BIN1, LOW);
  analogWrite(PWMB, SPEED);

  while (strcmp(v, right) == 0)
  {
    analogWrite(PWMB, SPEED);
    v = pRemoteCharacteristic->readValue().c_str();
    Serial.println("going to the right");
    Serial.println(SPEED);
  }
}

void stop_motor()
{
  is_off = true;
  digitalWrite(BIN2, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(LED, LOW);

  for (; SPEED > 0; SPEED -=20)
  {
    analogWrite(PWMA, SPEED);
    analogWrite(PWMB, SPEED);
    delay(200);
  }
  Serial.println("stop motor");
}
