#include <IRremote.h>
#include <IRremoteInt.h>
#include <ir_Lego_PF_BitStreamEncoder.h>
#include <boarddefs.h>

#include <DFRobot_Gesture.h>
#include <SparkFun_APDS9960.h>
#include <Wire.h>

// Global Variables
//SparkFun_APDS9960 apds = SparkFun_APDS9960();
//int isr_flag = 0;
//#define APDS9960_INT    2 // Needs to be an interrupt pin
int recvButtonVal = LOW;
int recvButtonPin = 2;
int testPin = 7;
unsigned char cmd;
// IR receiver
int RECV_PIN = 11;
IRrecv irrecv(RECV_PIN);
decode_results results;

unsigned long code;

// IR sender
IRsend irsend;
int sendButtonPin = 4;
int sendButtonVal = LOW;

//int TEST_RECV_PIN = 12;
//IRrecv testIrrecv(TEST_RECV_PIN);
//decode_results testResults;


long debounceDelay = 1000;
long lastButtonPress = 0;
long currButtonPress = 0;

DFRobot_Gesture myGesture;

void setup()
{
  // Set interrupt pin as input
//  pinMode(APDS9960_INT, INPUT);
  // Initialize interrupt service routine
//  attachInterrupt(0, interruptRoutine, FALLING);
//  if ( apds.enableGestureSensor(true) ) {
//    Serial.println(F("9960 Gesture sensor is now running"));
//  } else {
//    Serial.println(F("Something went wrong during 9960 gesture sensor init!"));
//  }
  // setup recvButton and sendButton
  pinMode(recvButtonPin, INPUT_PULLUP);
  pinMode(sendButtonPin, INPUT_PULLUP);
  // setup receiver
  irrecv.enableIRIn();

//  testIrrecv.enableIRIn();
  
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
//  pinMode(testPin, INPUT);
  Serial.write("Gesture sensors are now running....\n");
}


void loop()
{
//  if (testIrrecv.decode(&testResults)) {
//    Serial.println(results.value, HEX);
//    testIrrecv.resume();
//  }
  recvButtonVal = digitalRead(recvButtonPin);
  if (recvButtonVal == LOW) {
    currButtonPress = millis();
    if (currButtonPress - lastButtonPress > debounceDelay) {
      lastButtonPress = currButtonPress;
      handleRecvButton();
    }
  }
  sendButtonVal = digitalRead(sendButtonPin);
  if (sendButtonVal == LOW) {
    currButtonPress = millis();
    if (currButtonPress - lastButtonPress > debounceDelay) {
      lastButtonPress = currButtonPress;
      handleSendButton();
    }
  }

  
  // ADPS-9960
//  if( isr_flag == 1 ) {
//    detachInterrupt(0);
//    handleGesture();
//    isr_flag = 0;
//    attachInterrupt(0, interruptRoutine, FALLING);
//  }
  
//  if (digitalRead(testPin) == 0)
//  {
//    myGesture.I2C1_MasterRead(md.buf, 26, 0x42); //The address is:0x42
//    cmd = myGesture.mgcProcMsg();     //process the message
//    if (cmd != GI_NOGESTURE )
//    {
//      switch (cmd)
//      {
//
//        case GI_FLICK_R:
//          Serial.println("DF RIGHT");
//          break;
//
//        case GI_FLICK_L:
//          Serial.println("DF LEFT");
//          break;
//
//        case GI_FLICK_D:
//          Serial.println("DF DOWN");
//          break;
//
//        case GI_FLICK_U:
//          Serial.println("DF UP");
//          break;
//
//        case GI_AIRWHEEL_CW://Clockwise in circles
//          Serial.println("DF CW");
//          break;
//
//        case GI_AIRWHEEL_CCW://Counterclockwise circles
//          Serial.println("DF CCW");
//          break;
//
//        default: break;
//      }
//    }
//  }
//  else  {};
}

void handleRecvButton() {
  Serial.println("Recv button pressed.");
  bool received = false;
  while (!received) {
    if (irrecv.decode(&results)) {
      code = results.value;
      if (code == 0xFFFFFFFF) {
        Serial.println("REPEAT");
        irrecv.resume();
      }
      else {
        received = true;
        Serial.print("NEW CODE: ");
        Serial.println(code, HEX);
        irrecv.resume();
      }
      Serial.println(code, HEX);
    }
  }
}

void handleSendButton() {
  Serial.println("Send button pressed.");
  irsend.sendNEC(code, 32);
}

//// ADPS-9960
//void interruptRoutine() {
//  isr_flag = 1;
//}

//void handleGesture() {
//    if ( apds.isGestureAvailable() ) {
//    switch ( apds.readGesture() ) {
//      case DIR_UP:
//        Serial.println("UP");
//        break;
//      case DIR_DOWN:
//        Serial.println("DOWN");
//        break;
//      case DIR_LEFT:
//        Serial.println("LEFT");
//        break;
//      case DIR_RIGHT:
//        Serial.println("RIGHT");
//        break;
//      case DIR_NEAR:
//        Serial.println("NEAR");
//        break;
//      case DIR_FAR:
//        Serial.println("FAR");
//        break;
//      default:
//        Serial.println("NONE");
//    }
//  }
//}
