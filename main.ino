
#include "qrcode.h"
#include "Bitcoin.h"
#include "EEPROM.h"

const int touchPinNext = 15; 
const int touchPinPrev = 4; 
// change with your threshold value
const int threshold = 20;
// variable for storing the touch pin value 
int touchValueNext;
int touchValuePrev;

HDPublicKey pub("YOUR-MASTER-PUBKEY");

#define MAGIC 0x12345678 // some magic number to detect if we already wrote index in memory before
uint32_t ind = 0; // address index

void setup()
{
  Serial.begin(115200);

   // reading address index from memory
    EEPROM.begin(10);
  if(EEPROM.readULong(0) == MAGIC){ // magic, check if we stored index already
    ind = EEPROM.readULong(4);
  }else{ // memory is empty - write magic and index there
    EEPROM.writeULong(0, MAGIC);
    EEPROM.writeULong(4, ind);
    EEPROM.commit();
  }

  delay(100);
  showAddress();
}

void loop(){
  
   bool newaddr = false; // do we need to update?

  touchValueNext = touchRead(touchPinNext);
  touchValuePrev = touchRead(touchPinPrev);

  if((touchValueNext < 20) && (touchValueNext > 0)){ // next address
    ind++;
    newaddr = true;
  }

    
  if((touchValuePrev < 20) && (touchValuePrev > 0)){ // previous address
    ind--;
    newaddr = true;
  }
  if(newaddr){
    EEPROM.writeULong(4, ind); // write new index value
    EEPROM.commit();
    showAddress();
  }
 
  delay(100);
}

void showAddress(){

  String addr = pub.child(0).child(ind).address();
  Serial.println();
  Serial.println((String("bitcoin:")+addr).c_str());
  
  // auto detect best qr code size
  int qrSize = 10;
  int sizes[17] = { 14, 26, 42, 62, 84, 106, 122, 152, 180, 213, 251, 287, 331, 362, 412, 480, 504 };
  int len = addr.length();
  for(int i=0; i<17; i++){
    if(sizes[i] > len){
      qrSize = i+1;
      break;
    }
  }
  
  // Create the QR code
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(qrSize)];
  qrcode_initText(&qrcode, qrcodeData, qrSize, 1, (String("bitcoin:")+addr).c_str());

    // Top quiet zone
    Serial.print("\n\n\n\n");

    for (uint8_t y = 0; y < qrcode.size; y++) {

        // Left quiet zone
        Serial.print("        ");

        // Each horizontal module
        for (uint8_t x = 0; x < qrcode.size; x++) {

            // Print each module (UTF-8 \u2588 is a solid block)
            Serial.print(qrcode_getModule(&qrcode, x, y) ? "\u2588\u2588": "  ");
        }
        Serial.print("\n");
    }
    // Bottom quiet zone
    Serial.print("\n\n\n\n");
}
