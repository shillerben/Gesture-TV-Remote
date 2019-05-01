#include <IRremote.h>
#include <IRremoteInt.h>
#include <ir_Lego_PF_BitStreamEncoder.h>
#include <boarddefs.h>

#include <DFRobot_Gesture.h>
#include <SparkFun_APDS9960.h>
#include <Wire.h>


// Global Variables

// APDS-9960
SparkFun_APDS9960 apds = SparkFun_APDS9960();
#define APDS9960_INT    2 // Needs to be an interrupt pin

//// DF Robot
DFRobot_Gesture myGesture;
int testPin = 7;
unsigned char cmd;

// IR receiver
int RECV_PIN = 11;
IRrecv irrecv(RECV_PIN);
decode_results results;
unsigned long code;

// IR sender
IRsend irsend;

int recvButtonVal = LOW;
int recvButtonPin = 13;
int statusLed = 26;

// map from gesture to IR codes
// indices: DF_RIGHT, DF_LEFT, DF_UP, DF_DOWN, DF_CW, DF_CCW, APDS_RIGHT, APDS_LEFT, APDS_UP, APDS_DOWN, APDS_NEAR, APDS_FAR
long codes[12];

// debouncing
long debounceDelay = 1000;
long lastButtonPress = 0;
long currButtonPress = 0;


void setup()
{
  Serial.begin(9600);  // start serial for output

  // APDS setup
  // Set interrupt pin as input
  pinMode(APDS9960_INT, INPUT);
  // Initialize APDS-9960 (configure I2C and initial values)
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }
  // Start running the APDS-9960 gesture sensor engine
  if ( apds.enableGestureSensor(true) ) {
    Serial.println(F("9960 Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during 9960 gesture sensor init!"));
  }

  // testPin for DFRobot
  pinMode(testPin, INPUT);

  // setup recvButton and statusLed
  pinMode(recvButtonPin, INPUT_PULLUP);
  pinMode(statusLed, OUTPUT);

  // setup receiver
  irrecv.enableIRIn();

  Serial.write("Gesture sensors are now running....\n");
}


void loop()
{
  if (irrecv.decode(&results)) {
    Serial.print("Found random IR signal ");
    Serial.println(results.value, HEX);
    irrecv.resume(); // Receive the next value
  }
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
  if ( apds.isGestureAvailable() ) {
    handleGesture();
  }

  if (digitalRead(testPin) == 0) {
    myGesture.I2C1_MasterRead(md.buf, 26, 0x42); //The address is:0x42
    cmd = myGesture.mgcProcMsg();     //process the message
    if (cmd != GI_NOGESTURE ) {
      switch (cmd) {
        case GI_FLICK_R:
          Serial.println("DF RIGHT");
          Serial.print("sending code: ");
          Serial.println(codes[0], HEX);
          irsend.sendNEC(codes[0], 32);
          irrecv.enableIRIn();
          break;

        case GI_FLICK_L:
          Serial.println("DF LEFT");
          Serial.print("sending code: ");
          Serial.println(codes[1], HEX);
          irsend.sendNEC(codes[1], 32);
          irrecv.enableIRIn();
          break;

        case GI_FLICK_U:
          Serial.println("DF UP");
          Serial.print("sending code: ");
          Serial.println(codes[2], HEX);
          irsend.sendNEC(codes[2], 32);
          irrecv.enableIRIn();
          break;

        case GI_FLICK_D:
          Serial.println("DF DOWN");
          Serial.print("sending code: ");
          Serial.println(codes[3], HEX);
          irsend.sendNEC(codes[3], 32);
          irrecv.enableIRIn();
          break;

        case GI_AIRWHEEL_CW://Clockwise in circles
          Serial.println("DF CW");
          Serial.print("sending code: ");
          Serial.println(codes[4], HEX);
          irsend.sendNEC(codes[4], 32);
          irrecv.enableIRIn();
          break;

        case GI_AIRWHEEL_CCW://Counterclockwise circles
          Serial.println("DF CCW");
          Serial.print("sending code: ");
          Serial.println(codes[5], HEX);
          irsend.sendNEC(codes[5], 32);
          irrecv.enableIRIn();
          break;

        default: break;
      }
    }
  }
  else  {};
}

void updateCodes(unsigned char command, long code) {
  switch (command) {
    case GI_FLICK_R:
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

    case DIR_RIGHT + 10:
      codes[6] = code;
      break;

    case DIR_LEFT + 10:
      codes[7] = code;
      break;

    case DIR_UP + 10:
      codes[8] = code;
      break;

    case DIR_DOWN + 10:
      codes[9] = code;
      break;

    case DIR_NEAR + 10:
      codes[10] = code;
      break;

    case DIR_FAR + 10:
      codes[11] = code;
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
    else {};
  }
}

int getAPDSGesture() {
  int gesture = -1;
  while (!apds.isGestureAvailable()) {
    delay(10);
  }
  // got a gesture
  if (apds.isGestureAvailable()) {
    gesture = apds.readGesture() + 10;
  }
  Serial.print("Got APDS Gesture: ");
  Serial.println(gesture);
  return gesture;
}

unsigned char getGesture() {
  unsigned char command = 0;
  while (true) {
    // DF ROBOT
    if (digitalRead(testPin) == 0) {
      myGesture.I2C1_MasterRead(md.buf, 26, 0x42); //The address is:0x42
      command = myGesture.mgcProcMsg();     //process the message
      if (command != GI_NOGESTURE ) {
        return command;
      }
    }
    else if (apds.isGestureAvailable()) {
      command = apds.readGesture() + 10;
      return command;
    }
  }
}

void printGesture(unsigned char command) {
  switch (command) {
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

    case DIR_RIGHT + 10:
      Serial.println("APDS RIGHT");
      break;

    case DIR_LEFT + 10:
      Serial.println("APDS LEFT");
      break;

    case DIR_UP + 10:
      Serial.println("APDS UP");
      break;

    case DIR_DOWN + 10:
      Serial.println("APDS DOWN");
      break;

    case DIR_NEAR + 10:
      Serial.println("APDS NEAR");
      break;

    case DIR_FAR + 10:
      Serial.println("APDS FAR");
      break;

    default: break;
  }
}

void handleRecvButton() {
  Serial.println("Recv button pressed.");
  flushAPDS();
  flushDF();
  digitalWrite(statusLed, HIGH);
  Serial.println("Do a gesture");
  unsigned char command = getGesture();
  printGesture(command);
  digitalWrite(statusLed, LOW);
  bool received = false;
  flushIR();
  digitalWrite(statusLed, HIGH);
  Serial.println("Send IR signal");
  code = REPEAT;
  while (code == REPEAT) {
    if (irrecv.decode(&results)) {
      Serial.print("Found code::: ");
      Serial.println(results.value, HEX);
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
  digitalWrite(statusLed, LOW);
}

void handleGesture() {
  digitalWrite(statusLed, HIGH);
  switch ( apds.readGesture() ) {
    case DIR_RIGHT:
      Serial.println("APDS RIGHT");
      Serial.print("sending code: ");
      Serial.println(codes[6], HEX);
      irsend.sendNEC(codes[6], 32);
      irrecv.enableIRIn();
      break;
    case DIR_LEFT:
      Serial.println("APDS LEFT");
      Serial.print("sending code: ");
      Serial.println(codes[7], HEX);
      irsend.sendNEC(codes[7], 32);
      irrecv.enableIRIn();
      break;
    case DIR_UP:
      Serial.println("APDS UP");
      Serial.print("sending code: ");
      Serial.println(codes[8], HEX);
      irsend.sendNEC(codes[8], 32);
      irrecv.enableIRIn();
      break;
    case DIR_DOWN:
      Serial.println("APDS DOWN");
      Serial.print("sending code: ");
      Serial.println(codes[9], HEX);
      irsend.sendNEC(codes[9], 32);
      irrecv.enableIRIn();
      break;
    case DIR_NEAR:
      Serial.println("APDS NEAR");
      Serial.print("sending code: ");
      Serial.println(codes[10], HEX);
      irsend.sendNEC(codes[10], 32);
      irrecv.enableIRIn();
      break;
    case DIR_FAR:
      Serial.println("APDS FAR");
      Serial.print("sending code: ");
      Serial.println(codes[11], HEX);
      irsend.sendNEC(codes[11], 32);
      irrecv.enableIRIn();
      break;
    default:
      Serial.println("APDS NONE");
  }
  digitalWrite(statusLed, LOW);
}

void flushIR() {
  long long startTime = millis();
  while (millis() - startTime < 2000) {
    if (irrecv.decode(&results)) {
      Serial.print("Flushing: ");
      Serial.println(results.value, HEX);
      irrecv.resume();
    }
    delay(100);
  }
}

void flushAPDS() {
  long long startTime = millis();
  while (millis() - startTime < 1000) {
    if (apds.isGestureAvailable()) {
      apds.readGesture();
    }
    delay(100);
  }
}

void flushDF() {
  long long startTime = millis();
  while (millis() - startTime < 1000) {
    if (digitalRead(testPin) == 0) {
      myGesture.I2C1_MasterRead(md.buf,26,0x42); //The address is:0x42
      myGesture.mgcProcMsg();     //process the message
    }
    delay(100);
  }
}
