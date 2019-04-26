#include <IRremote.h>
#include <IRremoteInt.h>
#include <ir_Lego_PF_BitStreamEncoder.h>
#include <boarddefs.h>

#include <DFRobot_Gesture.h>
#include <SparkFun_APDS9960.h>
#include <Wire.h>


// Global Variables
SparkFun_APDS9960 apds = SparkFun_APDS9960();
int isr_flag = 0;
#define APDS9960_INT    2 // Needs to be an interrupt pin
int recvButtonVal = LOW;
int recvButtonPin = 3;
int testPin = 7;
unsigned char cmd;
// IR receiver
int RECV_PIN = 5;
IRrecv irrecv(RECV_PIN);
decode_results results;

unsigned long code;

// IR sender
IRsend irsend;
//int sendButtonPin = 4;
//int sendButtonVal = LOW;

//int TEST_RECV_PIN = 12;
//IRrecv testIrrecv(TEST_RECV_PIN);
//decode_results testResults;

// map from gesture to IR codes
// indices: DF_RIGHT, DF_LEFT, DF_UP, DF_DOWN, DF_CW, DF_CCW, APDS_RIGHT, APDS_LEFT, APDS_UP, APDS_DOWN, APDS_NEAR, APDS_FAR
long codes[12];

long debounceDelay = 1000;
long lastButtonPress = 0;
long currButtonPress = 0;

DFRobot_Gesture myGesture;

void setup()
{
  // Set interrupt pin as input
  pinMode(APDS9960_INT, INPUT);
  // Initialize interrupt service routine
  attachInterrupt(0, interruptRoutine, FALLING);
  if ( apds.enableGestureSensor(true) ) {
    Serial.println(F("9960 Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during 9960 gesture sensor init!"));
  }
  // setup recvButton and sendButton
  pinMode(recvButtonPin, INPUT_PULLUP);
//  pinMode(sendButtonPin, INPUT_PULLUP);
  // setup receiver
  irrecv.enableIRIn();

//  testIrrecv.enableIRIn();
  
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  // testPin for DFRobot
  pinMode(testPin, INPUT);
  Serial.write("Gesture sensors are now running....\n");
}


void loop()
{
  recvButtonVal = digitalRead(recvButtonPin);
  if (recvButtonVal == LOW) {
    currButtonPress = millis();
    if (currButtonPress - lastButtonPress > debounceDelay) {
      Serial.println("button pushed");
      lastButtonPress = currButtonPress;
      handleRecvButton();
    }
  }
  
  // ADPS-9960
  if( isr_flag == 1 ) {
    detachInterrupt(0);
    handleGesture();
    isr_flag = 0;
    attachInterrupt(0, interruptRoutine, FALLING);
  }
  
  if (digitalRead(testPin) == 0)
  {
    myGesture.I2C1_MasterRead(md.buf, 26, 0x42); //The address is:0x42
    cmd = myGesture.mgcProcMsg();     //process the message
    if (cmd != GI_NOGESTURE )
    {
      switch (cmd)
      {

        case GI_FLICK_R:
          Serial.println("DF RIGHT");
          Serial.print("sending code: ");
          Serial.println(codes[0], HEX);
          irsend.sendNEC(codes[0], 32);
          break;

        case GI_FLICK_L:
          Serial.println("DF LEFT");
          Serial.print("sending code: ");
          Serial.println(codes[1], HEX);
          irsend.sendNEC(codes[1], 32);
          break;

        case GI_FLICK_U:
          Serial.println("DF UP");
          Serial.print("sending code: ");
          Serial.println(codes[2], HEX);
          irsend.sendNEC(codes[2], 32);
          break;

        case GI_FLICK_D:
          Serial.println("DF DOWN");
          Serial.print("sending code: ");
          Serial.println(codes[3], HEX);
          irsend.sendNEC(codes[3], 32);
          break;

        case GI_AIRWHEEL_CW://Clockwise in circles
          Serial.println("DF CW");
          Serial.print("sending code: ");
          Serial.println(codes[4], HEX);
          irsend.sendNEC(codes[4], 32);
          break;

        case GI_AIRWHEEL_CCW://Counterclockwise circles
          Serial.println("DF CCW");
          Serial.print("sending code: ");
          Serial.println(codes[5], HEX);
          irsend.sendNEC(codes[5], 32);
          break;

        default: break;
      }
    }
  }
  else  {};
}

void updateCodes(unsigned char command, long code) {
  switch(command) {
    case GI_FLICK_R:
      Serial.print("codes[0] = ");
      Serial.println(codes[0], HEX);
      codes[0] = code;
      break;

    case GI_FLICK_L:
      codes[1] = code;
      break;

    case GI_FLICK_U:
      codes[2] = code;
      break;

    case GI_FLICK_D:
      codes[3] = code;
      break;

    case GI_AIRWHEEL_CW://Clockwise in circles
      codes[4] = code;
      break;

    case GI_AIRWHEEL_CCW://Counterclockwise circles
      codes[5] = code;
      break;

    default: break;
  }
}

unsigned char getDFRobotGesture() {
  unsigned char command = 0;
  while (true) {
    if (digitalRead(testPin) == 0) {
      myGesture.I2C1_MasterRead(md.buf, 26, 0x42); //The address is:0x42
      command = myGesture.mgcProcMsg();     //process the message
      if (command != GI_NOGESTURE ) {
        return command;
      }
    }
  }
}

int getAPDSGesture() {
  int gesture = -1;
  while (isr_flag == 0) {
    delay(10);
  }
  // got a gesture
  if (apds.isGestureAvailable()) {
    gesture = apds.readGesture();
  }
  Serial.print("Got APDS Gesture: ");
  Serial.println(gesture);
  return gesture;
}

void printGesture(unsigned char command) {
  switch (command){
    case GI_FLICK_R:
      Serial.println("DF RIGHT");
      break;

    case GI_FLICK_L:
      Serial.println("DF LEFT");
      break;

    case GI_FLICK_U:
      Serial.println("DF UP");
      break;

    case GI_FLICK_D:
      Serial.println("DF DOWN");
      break;

    case GI_AIRWHEEL_CW://Clockwise in circles
      Serial.println("DF CW");
      break;

    case GI_AIRWHEEL_CCW://Counterclockwise circles
      Serial.println("DF CCW");
      break;

    default: break;
  }
}

void handleRecvButton() {
  Serial.println("Recv button pressed.");
  Serial.println("Do a gesture");
  unsigned char command = getDFRobotGesture();
  printGesture(command);
  bool received = false;
  code = REPEAT;
  while (code == REPEAT) {
    if (irrecv.decode(&results)) {
      if (results.value == REPEAT) {
        irrecv.resume();
      }
      else {
        Serial.println(results.value, HEX);
        irrecv.resume();
        code = results.value;
      }
    }
    delay(10);
  }
  Serial.print("NEW CODE: ");
  Serial.println(code, HEX);
  updateCodes(command, code);
}

void handleSendButton() {
  Serial.println("Send button pressed.");
  irsend.sendNEC(code, 32);
}

// APDS-9960
void interruptRoutine() {
  isr_flag = 1;
}

void handleGesture() {
    if ( apds.isGestureAvailable() ) {
    switch ( apds.readGesture() ) {
      case DIR_RIGHT:
        Serial.println("APDS RIGHT");
        Serial.print("sending code: ");
        Serial.println(codes[6], HEX);
        irsend.sendNEC(codes[6], 32);
        break;
      case DIR_LEFT:
        Serial.println("APDS LEFT");
        Serial.print("sending code: ");
        Serial.println(codes[7], HEX);
        irsend.sendNEC(codes[7], 32);
        break;
      case DIR_UP:
        Serial.println("APDS UP");
        Serial.print("sending code: ");
        Serial.println(codes[8], HEX);
        irsend.sendNEC(codes[8], 32);
        break;
      case DIR_DOWN:
        Serial.println("APDS DOWN");
        Serial.print("sending code: ");
        Serial.println(codes[9], HEX);
        irsend.sendNEC(codes[9], 32);
        break;
      case DIR_NEAR:
        Serial.println("APDS NEAR");
        Serial.print("sending code: ");
        Serial.println(codes[10], HEX);
        irsend.sendNEC(codes[10], 32);
        break;
      case DIR_FAR:
        Serial.println("APDS FAR");
        Serial.print("sending code: ");
        Serial.println(codes[11], HEX);
        irsend.sendNEC(codes[11], 32);
        break;
      default:
        Serial.println("APDS NONE");
    }
  }
}
