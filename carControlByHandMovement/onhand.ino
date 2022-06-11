#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Wire.h>

#define LED 26
#define SCL 22
#define SDA 21
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
BLECharacteristic *pCharacteristic;
BLEServer *pServer;

TwoWire I2C = TwoWire(0);
const int MPU = 0x68;
int count_rotate_left = 0, count_rotate_right = 0, count_rotate_down = 0, count_rotate_up = 0, count_activate = 0;
bool right_rotated, left_rotated, up_rotated, down_rotated, is_on = false;
char* message;

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  
  Serial.println("Starting BLE work!");

  BLEDevice::init("onHand");
  pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setValue("520");
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x12);
  
    BLEDevice::startAdvertising();

  
  Serial.println("Characteristic defined! Now you can read it in your phone!");

  I2C.begin(SDA, SCL);
  Wire.begin(SDA, SCL);                      
  Wire.beginTransmission(MPU);       
  Wire.write(0x6B);                  
  Wire.write(0x00);                  
  Wire.endTransmission(true);
}


void loop() {

  pServer->startAdvertising();
  delay(100);
  
  float GyroX = 0, GyroY = 0, GyroZ;
  Serial.println();
  
  int iterate = 175;
  for (int i=0; i<iterate; i++)
  {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 4, true);
    GyroX += (Wire.read() << 8 | Wire.read()) / 131.0;
    GyroY += (Wire.read() << 8 | Wire.read()) / 131.0;
  }

  GyroX /= iterate;
  GyroY /= iterate;
  Serial.printf(" GyroX:%f", GyroX);
  Serial.printf(" GyroY:%f", GyroY);

  if (GyroX > 240 && GyroX <280)
  {
    count_activate++;
    if (count_activate > 3)
    {
      if (!is_on)
      {
        is_on = true;
        count_activate = 0;

        pCharacteristic->setValue("forward");
        Serial.println("message 'forward' is sent");
        message = "forward";
        digitalWrite(LED, HIGH);
      }
      else
      {
        is_on = false;
        count_activate = 0;

        pCharacteristic->setValue("off");
        Serial.println("message 'off' is sent");
        left_rotated = false;
        right_rotated = false;
        digitalWrite(LED, LOW);
      }
    }
  }
  else
  {
    count_activate = 0;
  }

  if (is_on)
  {
      Serial.printf(" count_activate:%d", count_activate);
    
      //xu li xoay xuong
      if (GyroY > 8 && GyroY < 95)
      {
        count_rotate_down++;
        if (count_rotate_down > 4)
        {
            pCharacteristic->setValue("up");
            delay(50);
            pCharacteristic->setValue(message);
            Serial.println("message 'up' is sent");
            digitalWrite(LED, HIGH);
            count_rotate_left = 0;
            count_rotate_right = 0;
            GyroX = 0;
        }
      }
    
      //xu li xoay len
      else if (GyroY > 400)
      {
        count_rotate_up++;
        if (count_rotate_up > 4)
        {
            pCharacteristic->setValue("down");
            delay(50);
            pCharacteristic->setValue(message);
            Serial.println("message 'down' is sent");
            digitalWrite(LED, LOW);
            count_rotate_left = 0;
            count_rotate_right = 0;
            GyroX = 0;
        }
      }
      //loc tin hieu bat on dinh cua GyroY
      else
      {
        count_rotate_down = 0;
        count_rotate_up = 0;
      }

      //xu li xoay phai
      if (GyroX > 30 && GyroX < 200)
      {
        count_rotate_right++;
        if (count_rotate_right > 3)
        {
          count_rotate_down = 0;
          count_rotate_up = 0;
          //xoay tu trung tam sang phai
          if (!left_rotated)
          {
            right_rotated = true;
            pCharacteristic->setValue("right");
            message = "right";
            Serial.println("message 'right' is sent");
            digitalWrite(LED, HIGH);
          }
          //xoay tu trai ve trung tam
          else
          {
            left_rotated = false;
            count_rotate_right = 0;
            
            pCharacteristic->setValue("forward");
            message = "forward";
            Serial.println("message 'forward' is sent");
            digitalWrite(LED, LOW);
          }
        }
      }
      
      //xu li xoay trai
      else if (GyroX > 350)
      {
        count_rotate_left++;
        if (count_rotate_left > 5)
        {
          count_rotate_down = 0;
          count_rotate_up = 0;
          //xoay trung tam sang phai
          if (!right_rotated)
          {
            left_rotated = true;
    
            pCharacteristic->setValue("left");
            Serial.println("message 'left' is sent");
            message = "left";
            digitalWrite(LED, HIGH);
          }
    
          //xoay tu phai ve trung tam
          else
          {
            right_rotated = false;
            count_rotate_left = 0;
            
            digitalWrite(LED, LOW);
            pCharacteristic->setValue("forward");
            message = "forward";
            Serial.println("message 'forward' is sent");
          }
        }
      }
      //loc tin hieu bat on dinh
      else
      {
        count_rotate_left = 0;
        count_rotate_right = 0;
      }
    //  
    //  Serial.printf(" count_rotate_right:%d count_rotate_left:%d", count_rotate_right, count_rotate_left);
      Serial.printf (" count_rotate_down:%d count_rotate_up:%d", count_rotate_down, count_rotate_up);
  }
}
