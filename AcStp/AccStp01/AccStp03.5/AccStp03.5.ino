#include <AccelStepper.h>

#define DIR   8 //PB0
#define STEP  7//PD7
#define EN    3 //PD3
#define m0    4 //PORTD4
#define m1    5 //PORTD5
#define m2    6 //PORTD6

AccelStepper stepper (AccelStepper::DRIVER,STEP,DIR);
void setup(){  
  Serial.begin(115200);
  pinMode(EN,OUTPUT);
  pinMode( m0 , OUTPUT );
  pinMode( m1 , OUTPUT );
  pinMode( m2 , OUTPUT );
  digitalWrite(m0 , HIGH);
  digitalWrite(m1 , HIGH);
  digitalWrite(m2 , HIGH);
  digitalWrite(EN,LOW);
  stepper.setMaxSpeed(1800);
  stepper.setAcceleration(800); 
}

void loop(){  
  stepper.move(3600);  
  while(stepper.distanceToGo()>0){
  Serial.println(stepper.distanceToGo());
  stepper.run();
  }
  delay(1000);
}
