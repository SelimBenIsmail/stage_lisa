#include <AccelStepper.h>

#define DIR 8 //PB0
#define STEP 7//PD7
#define EN 3 //PD3

AccelStepper stepper (AccelStepper::DRIVER,STEP,DIR);

int i;
void setup()
{ 
  Serial.begin(115200);
  pinMode(EN,OUTPUT);
  digitalWrite(EN,LOW);
  stepper.setMaxSpeed(1000);
  stepper.setSpeed(50);  
}

void loop()
{  

   stepper.runSpeed();
   
}
