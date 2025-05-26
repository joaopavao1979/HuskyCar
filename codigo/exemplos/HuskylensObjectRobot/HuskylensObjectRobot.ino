//The DIY Life
//Michael Klements
//16/04/2021
//HuskyLens Object Tracking Robot

#include "HUSKYLENS.h"                            //Import the required libraries
#include "SoftwareSerial.h"
#include <AFMotor.h>

AF_DCMotor rightBack(1);                          //Create an object to control each motor
AF_DCMotor rightFront(2);
AF_DCMotor leftFront(3);
AF_DCMotor leftBack(4);

byte maxMotorSpeed = 50;                             //The normal motor speed
int motorOffset = 25;                             //Factor to account for one side being more powerful
int objectWidth = 50;
int turnGain = 12;
int offCentre = 20;
int distGain = 6;

int leftLimit = 160-offCentre;
int rightLimit = 160+offCentre;

int leftSp = 0;
int rightSp = 0;

bool isTurning = false;                           //Track when turning left and right
bool isTurningLeft = true;

HUSKYLENS huskylens;                              //Create a new Huskeylens object
int ID1 = 1;

void printResult(HUSKYLENSResult result);

void setup() 
{
  Serial.begin(115200);                                       //Start serial communication
  Wire.begin();                                               //Connect to Huskylens
  while (!huskylens.begin(Wire))
  {
      Serial.println(F("Begin failed!"));
      Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protol Type>>I2C)"));
      Serial.println(F("2.Please recheck the connection."));
      delay(100);
  }
  huskylens.writeAlgorithm(ALGORITHM_OBJECT_TRACKING);        //Switch the algorithm to object tracking.
  rightBack.setSpeed(0);                                      //Set the motors to the motor speed
  rightFront.setSpeed(0);
  leftFront.setSpeed(0);
  leftBack.setSpeed(0);
  moveForward();                                              //Set motors to forward direction
}

void loop() 
{
  int32_t error;
  if (!huskylens.request())                                                 //If connection to Huskylens isn't available
    Serial.println(F("Fail to request objects from HUSKYLENS!"));
  else if(!huskylens.isLearned())                                           //If an object hasn't yet been learned
  {
    Serial.println(F("Object not learned!")); 
    setMotorSpeed (0,0);
  }
  else if(!huskylens.available())                                           //If the learned object isn't visible anymore
  {
    Serial.println(F("Object disappeared!"));
    setMotorSpeed (0,0);
  }
  else                                                                      //If object is being tracked
  {
     HUSKYLENSResult result = huskylens.read();                             //Get the results of the object being tracked
     if(result.xCenter < leftLimit)
     {
        leftSp = -turnGain*(leftLimit-result.xCenter);
        rightSp = turnGain*(leftLimit-result.xCenter);
     }
     else if(result.xCenter > rightLimit)
     {
        leftSp = turnGain*(result.xCenter-rightLimit);
        rightSp = -turnGain*(result.xCenter-rightLimit);
     }
     if(result.width < objectWidth)
     {
        leftSp = leftSp+(distGain*(objectWidth-result.width));
        rightSp = rightSp+(distGain*(objectWidth-result.width));
     }
     if(leftSp > maxMotorSpeed)
        leftSp = maxMotorSpeed;
     else if(leftSp < 0)
        leftSp = 0;
     if(rightSp > maxMotorSpeed)
        rightSp = maxMotorSpeed;
     else if(rightSp < 0)
        rightSp = 0;
     setMotorSpeed (leftSp, rightSp);
     leftSp = 0;
     rightSp = 0;
     printResult(result);
  }
}

void moveForward()                                //Set all motors to run forward
{
  rightBack.run(FORWARD);
  rightFront.run(FORWARD);
  leftFront.run(FORWARD);
  leftBack.run(FORWARD);
}

void stopMove()                                   //Set all motors to stop
{
  rightBack.run(RELEASE);
  rightFront.run(RELEASE);
  leftFront.run(RELEASE);
  leftBack.run(RELEASE);
}

void setMotorSpeed (int leftTempSp, int rightTempSp)
{
  rightBack.setSpeed(rightTempSp);                              //Set the motors to the motor speed
  rightFront.setSpeed(rightTempSp);
  leftFront.setSpeed(leftTempSp+motorOffset);
  leftBack.setSpeed(leftTempSp+motorOffset);
}

void printResult(HUSKYLENSResult result)                        //Display the results on the serial monitor
{
    if (result.command == COMMAND_RETURN_BLOCK)
    {
        Serial.println(String()+F("Block:xCenter=")+result.xCenter+F(",yCenter=")+result.yCenter+F(",width=")+result.width+F(",height=")+result.height+F(",ID=")+result.ID);
    }
    else if (result.command == COMMAND_RETURN_ARROW)
    {
        Serial.println(String()+F("Arrow:xOrigin=")+result.xOrigin+F(",yOrigin=")+result.yOrigin+F(",xTarget=")+result.xTarget+F(",yTarget=")+result.yTarget+F(",ID=")+result.ID);
    }
    else
    {
        Serial.println("Object unknown!");
    }
}
