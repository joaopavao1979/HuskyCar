//The DIY Life
//Michael Klements
//16/04/2021
//HuskyLens Line Tracking Robot
//Adapted from Huskylens Line Tracking example by By [Angelo qiao](Angelo.qiao@dfrobot.com)

#include "HUSKYLENS.h"                            //Import the required libraries
#include "SoftwareSerial.h"
#include "PIDLoop.h"
#include <AFMotor.h>

AF_DCMotor rightBack(1);                          //Create an object to control each motor
AF_DCMotor rightFront(2);
AF_DCMotor leftFront(3);
AF_DCMotor leftBack(4);

byte motorSpeed = 60;                             //The maximum motor speed
int motorOffset = 15;                             //Factor to account for one side being more powerful
int turnSpeed = 50;                               //Amount to add to motor speed when turning
int leftSpeed = 0;                                //Variables to keep current left and right motor speeds
int rightSpeed = 0;

PIDLoop headingLoop(120, 0, 0, false);            //Set up PID control for the heading, only P used for now
HUSKYLENS huskylens;                              //Create a Huskeylens object
int ID1 = 1;                                      //We're tracking recognised object 1

void printResult(HUSKYLENSResult result);

void setup() 
{
  Serial.begin(115200);                                         //Start serial communication
  Wire.begin();                                                 //Begin communication with the Huskeylens
  while (!huskylens.begin(Wire))
  {
      Serial.println(F("Begin failed!"));
      Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protol Type>>I2C)"));
      Serial.println(F("2.Please recheck the connection."));
      delay(100);
  }
  huskylens.writeAlgorithm(ALGORITHM_LINE_TRACKING);            //Switch the algorithm to line tracking.
  rightBack.setSpeed(0);                                        //Set the motors to the motor speed, initially all 0
  rightFront.setSpeed(0);
  leftFront.setSpeed(0);
  leftBack.setSpeed(0);
  moveForward();                                                //Start the motors
  accelerate();                                                 //Accelerate to the set motor speed
}

void loop() 
{
  int32_t error;
  if (!huskylens.request(ID1))                                  //If a connection is not established
  {
    Serial.println(F("Check connection to Huskeylens"));
    leftSpeed = 0; 
    rightSpeed = 0;
  }
  else if(!huskylens.isLearned())                               //If an object has not been learned
  {
    Serial.println(F("No object has been learned"));
    leftSpeed = 0; 
    rightSpeed = 0;
  }
  else if(!huskylens.available())                               //If there is no arrow being shown on the screen yet
    Serial.println(F("No arrow on the screen yet"));
  else                                                          //Once a line is detected and an arrow shown
  {
    HUSKYLENSResult result = huskylens.read();                  //Read and display the result
    printResult(result);
    
    error = (int32_t)result.xTarget - (int32_t)160;             //Calculate the tracking error
    headingLoop.update(error);                                  //Perform PID control on the error
       
    leftSpeed = headingLoop.m_command;                          //Get the left side speed change
    rightSpeed = -headingLoop.m_command;                        //Get the right side speed change

    leftSpeed += motorSpeed;                      //Calculate the total left side speed
    rightSpeed += (motorSpeed-motorOffset);                                   //Calculate the total right side speed
  }
  Serial.println(String()+leftSpeed+","+rightSpeed);
  leftFront.setSpeed(leftSpeed);                                //Set the new motor speeds
  leftBack.setSpeed(leftSpeed); 
  rightFront.setSpeed(rightSpeed);
  rightBack.setSpeed(rightSpeed);
}

void accelerate()                                 //Function to accelerate the motors from 0 to full speed
{
  for (int i=0; i<motorSpeed; i++)                //Loop from 0 to full speed
  {
    rightBack.setSpeed(i);                        //Set the motors to the current loop speed
    rightFront.setSpeed(i);
    leftFront.setSpeed(i+motorOffset);
    leftBack.setSpeed(i+motorOffset);
    delay(10);
  }
}

void decelerate()                                 //Function to decelerate the motors from full speed to zero
{
  for (int i=motorSpeed; i!=0; i--)               //Loop from full speed to 0
  {
    rightBack.setSpeed(i);                        //Set the motors to the current loop speed
    rightFront.setSpeed(i);
    leftFront.setSpeed(i+motorOffset);
    leftBack.setSpeed(i+motorOffset); 
    delay(10);
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

void turnLeft(int duration)                                 //Set motors to turn left for the specified duration then continue
{
  rightBack.setSpeed(motorSpeed+turnSpeed);                 //Set the motors to the motor speed
  rightFront.setSpeed(motorSpeed+turnSpeed);
  leftFront.setSpeed(motorSpeed+motorOffset+turnSpeed);
  leftBack.setSpeed(motorSpeed+motorOffset+turnSpeed);
  rightBack.run(FORWARD);
  rightFront.run(FORWARD);
  leftFront.run(BACKWARD);
  leftBack.run(BACKWARD);
  delay(duration);
  rightBack.setSpeed(motorSpeed);                           //Set the motors to the motor speed
  rightFront.setSpeed(motorSpeed);
  leftFront.setSpeed(motorSpeed+motorOffset);
  leftBack.setSpeed(motorSpeed+motorOffset);
  moveForward();
}

void turnRight(int duration)                                //Set motors to turn right for the specified duration then continue
{
  rightBack.setSpeed(motorSpeed+turnSpeed);                 //Set the motors to the motor speed
  rightFront.setSpeed(motorSpeed+turnSpeed);
  leftFront.setSpeed(motorSpeed+motorOffset+turnSpeed);
  leftBack.setSpeed(motorSpeed+motorOffset+turnSpeed);
  rightBack.run(BACKWARD);
  rightFront.run(BACKWARD);
  leftFront.run(FORWARD);
  leftBack.run(FORWARD);
  delay(duration);
  rightBack.setSpeed(motorSpeed);                           //Set the motors to the motor speed
  rightFront.setSpeed(motorSpeed);
  leftFront.setSpeed(motorSpeed+motorOffset);
  leftBack.setSpeed(motorSpeed+motorOffset);
  moveForward();
}

void printResult(HUSKYLENSResult result)                    //Display the results on the serial monitor
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
