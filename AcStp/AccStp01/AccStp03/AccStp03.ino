
#include <AccelStepper.h>

#define DIR   8 //PB0
#define STEP  7//PD7
#define EN    3 //PD3

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


MStepper turntableMotor; //nouvel objet turntablemotor de la classe MStepper

//Déclaration des variables
float mspeed=1800;
float sspeed= 150;
float accel=800;

//initialisation
void setup() {
  Serial.begin(115200);
  pinMode(EN,OUTPUT);
  digitalWrite(EN,LOW);

  turntableMotor.setMaxSpeed(mspeed);
  turntableMotor.setAcceleration(accel);
  turntableMotor.setSpeed(sspeed); 
  //turntableMotor.moveTo(1500);

}

void loop() {
  
  turntableMotor.runSpeed(); 
 // delay(5000);

}
