#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

RF24 radio(10, 9);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001";

unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;
int maxTimeLift = 255;
int maxTimeThrust = 118;
int temp = 0;

/* Declare pin numbers for output */
int thrustPinOne = 18; 
int thrustPinTwo = 6; 
int liftPinOne = 1;
int liftPinTwo = 2; 
int thrustOneDir = 3;
int thrustTwoDir = 4;
int liftOneDir = 7;
int liftTwoDir = 8;
int servoOnePin = 5;
int servoTwoPin = 6;

Servo servoOne;
Servo servoTwo;

// Max size of this struct is 32 bytes - NRF24L01 buffer limit
struct Data_Package {
  byte j1PotX;
  byte j1PotY;
  byte j1Button;
  byte j2PotX;
  byte j2PotY;
  byte j2Button;
  byte pot1;
  byte pot2;
  byte tSwitch1;
  byte tSwitch2;
  byte button1;
  byte button2;
  byte button3;
  byte button4;
};

Data_Package data; //Create a variable with the above structure

// -------------------------------------------------- BEGIN SETUP --------------------------------------------------

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening(); //  Set the module as receiver
  resetData();
  
  // Declare output pin for motors
  pinMode(thrustPinOne, OUTPUT);
  pinMode(thrustPinTwo, OUTPUT);
  pinMode(liftPinOne, OUTPUT);
  pinMode(liftPinTwo, OUTPUT);
  pinMode(thrustOneDir, OUTPUT);
  pinMode(thrustTwoDir, OUTPUT);
  pinMode(liftOneDir, OUTPUT);
  pinMode(liftTwoDir, OUTPUT);
  servoOne.attach(servoOnePin);
  servoTwo.attach(servoTwoPin);

  digitalWrite(liftOneDir, LOW);
  digitalWrite(liftTwoDir, LOW);
}

// -------------------------------------------------- END SETUP --------------------------------------------------

// -------------------------------------------------- BEGIN LOOP --------------------------------------------------

void loop() {
  // Check whether there is data to be received
  if (radio.available()) {
    radio.read(&data, sizeof(Data_Package)); // Read the whole data and store it into the 'data' structure
    lastReceiveTime = millis(); // At this moment we have received the data
  }
  // Check whether we keep receving data, or we have a connection between the two modules
  currentTime = millis();
  if ( currentTime - lastReceiveTime > 1000 ) { // If current time is more then 1 second since we have recived the last data, that means we have lost connection
    resetData(); // If connection is lost, reset the data. It prevents unwanted behavior, for example if a drone has a throttle up and we lose connection, it can keep flying unless we reset the values
  }
  // Print the data in the Serial Monitor
  Serial.print("j1PotX: ");
  Serial.print(data.j1PotX);
  Serial.print("; j1PotY: ");
  Serial.print(data.j1PotY);
  Serial.print("; button1: ");
  Serial.print(data.button1);
  Serial.print("; j2PotX: ");
  Serial.println(data.j2PotX); 

  /* Start of Implementation Code */

  //Potentiometer controlled lift fans
  
  if(!data.tSwitch1) {
    digitalWrite(liftPinOne, HIGH);
    delayMicroseconds(data.pot1); 
    digitalWrite(liftPinOne, LOW);
    delayMicroseconds(maxTimeLift - data.pot1);
  }

  //Joystick one 
  if(data.j1PotY > 137) {
    digitalWrite(thrustOneDir, LOW);
    temp = data.j1PotY - 137;
    digitalWrite(thrustPinOne, HIGH);
    delayMicroseconds(temp); 
    digitalWrite(thrustPinOne, LOW);
    delayMicroseconds(maxTimeThrust - temp);
  } else if(data.j1PotY < 117) {
    digitalWrite(thrustOneDir, HIGH);
    digitalWrite(thrustPinOne, LOW);
    delayMicroseconds(maxTimeThrust - data.j1PotY); 
    digitalWrite(thrustPinOne, HIGH);
    delayMicroseconds(data.j1PotY);
  }

  //Joystick  two
  if(data.j2PotY > 137) {
    digitalWrite(thrustTwoDir, LOW);
    temp = data.j2PotY - 137;
    digitalWrite(thrustPinTwo, HIGH);
    delayMicroseconds(temp); 
    digitalWrite(thrustPinTwo, LOW);
    delayMicroseconds(maxTimeThrust - temp);
  } else if(data.j2PotY < 117) {
    digitalWrite(thrustTwoDir, HIGH);
    digitalWrite(thrustPinTwo, LOW);
    delayMicroseconds(maxTimeThrust - data.j2PotY); 
    digitalWrite(thrustPinTwo, HIGH);
    delayMicroseconds(data.j2PotY);
  }

  //Activate Servo's
  if(!data.button1) {
    servoOne.write(90);
  }

  if(!data.button2) {
    servoTwo.write(90);
  }

  
}

// -------------------------------------------------- END LOOP --------------------------------------------------

void resetData() {
  // Reset the values when there is no radio connection - Set initial default values
  data.j1PotX = 127;
  data.j1PotY = 127;
  data.j2PotX = 127;
  data.j2PotY = 127;
  data.j1Button = 1;
  data.j2Button = 1;
  data.pot1 = 1;
  data.pot2 = 1;
  data.tSwitch1 = 1;
  data.tSwitch2 = 1;
  data.button1 = 1;
  data.button2 = 1;
  data.button3 = 1;
  data.button4 = 1;
}
