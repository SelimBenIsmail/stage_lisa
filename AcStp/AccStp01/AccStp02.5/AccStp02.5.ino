#include <AccelStepper.h>

#define DIR 8 //PB0
#define STEP 7//PD7
#define EN 3 //PD3

AccelStepper stepper (AccelStepper::DRIVER,STEP,DIR);
void setup()
{  
}

void loop()
{
    if (stepper.distanceToGo() == 0)
    {
  // Random change to speed, position and acceleration
  // Make sure we dont get 0 speed or accelerations
  delay(1000);
  stepper.moveTo(1440);
  stepper.setMaxSpeed(1800);
  stepper.setAcceleration(800);
    }
    stepper.run();
}
