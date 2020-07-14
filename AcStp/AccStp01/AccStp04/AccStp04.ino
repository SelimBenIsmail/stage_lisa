#include <AccelStepper.h>

#define DIR   8 //PB0
#define STEP  7//PD7
#define EN    3 //PD3
#define m0    4 //PORTD4
#define m1    5 //PORTD5
#define m2    6 //PORTD6

class MStepper: public AccelStepper
{ 
  public:
    MStepper():AccelStepper(AccelStepper::DRIVER,STEP,DIR) {}
    void stop1() {
      if (_direction == DIRECTION_CW) moveTo(currentPosition() + 1600);
      else moveTo(currentPosition() - 1600);
    }
  protected:
   /* void step(long step) {
      if (_direction) PORTB = PORTB | 0x01;
      else PORTB = PORTB & 0xFE;
      PORTD = PORTD | 0x80;
      delayMicroseconds(2);
      PORTD = PORTD & 0x7F;
    }*/

};


MStepper turntableMotor;
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
  turntableMotor.setMaxSpeed(1800);
  turntableMotor.setAcceleration(800); 
}

void loop(){  
  turntableMotor.move(3600);  
  while(turntableMotor.distanceToGo()>0){
  Serial.println(turntableMotor.distanceToGo());
  turntableMotor.run();
  }
  delay(1000);
}
