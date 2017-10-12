#include<SoftwareSerial.h>
#include<XBee.h>
#include<MFRC522.h>
#include<SPI.h>

#define SS_PIN 10
#define RSI_PIN 9

MFRC522 mfrc522(SS_PIN, RSI_PIN); // Create MFRC522 instance

const int sensorPin = A0;
const int nodeID = 6;
SoftwareSerial mySerial(2, 3);
XBee xbee = XBee();
Rx16Response rx16 = Rx16Response();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init(); // Initate MFRC522
  xbee.setSerial(mySerial);
  mySerial.begin(9600);
  Serial.println("Approximate your card to the reader...");
  Serial.println();
}

bool r1 = false;
bool r2 = false;
bool r3 = false;

String r1Rssi = "";
String r2Rssi = "";
String r3Rssi = "";

bool status = false;
String currentRFID = "none";
int weight = 0;

void loop() {
  // put your main code here, to run repeatedly:
  
  xbee.readPacket();
  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
      xbee.getResponse().getRx16Response(rx16);
      uint8_t str_rssi = rx16.getRssi();
      String r = "";
      for (int i = 0; i < rx16.getDataLength(); i++) {
        r += (char) rx16.getData(i);
      }
//      if (r.equals("res")) {
      Serial.println("Received from " + (String) rx16.getRemoteAddress16() + ", rssi = " + str_rssi);
      int rssi = (int) str_rssi;
      Serial.println(r);
      switch(rx16.getRemoteAddress16()) {
        case 1: 
          r1 = true;
          r1Rssi = str_rssi;
          break;
        case 2: 
          r2 = true;
          r2Rssi = str_rssi;
          break;
        case 3: 
          r3 = true;
          r3Rssi = str_rssi;
          break;
      }
//      }
    }
    if (r1 == true && r2 == true && r3 == true) {
      weight = analogRead(sensorPin);
      weight = map(weight, 0, 1023, 0, 255);
      if (weight != 0) {
        status = true;
      } else {
        status = false;
      }
      String req = "{\"r1\": " + (String) r1Rssi + ", \"r2\": " + (String) r2Rssi + ", \"r3\": " 
                  + (String) r3Rssi + ", \"nodeID\": " + (String) nodeID + ", \"status\": " + status + ", \"currentRFID\": \"" + currentRFID + "\"}";
      Serial.println(req);
      int len = req.length();
      uint8_t payload[len];
      for (int i = 0; i < len; i++) {
        payload[i] = (uint8_t) req[i];
      }
      Tx16Request tx16 = Tx16Request(5, payload, sizeof(payload));
      xbee.send(tx16);
      Serial.println("Sent to 5");
      r1 = false;
      r2 = false;
      r3 = false;
    }
  }
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      //Show UID on serial monitor
      String content = "";
      byte letter;
      for (byte i = 0; i < mfrc522.uid.size; i++) 
      {
         content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
         content.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      Serial.print("UID tag : ");
      content.toUpperCase();
      Serial.println(content);
      status = true;
      currentRFID = content;
      delay(2000);
    }
  }
}









