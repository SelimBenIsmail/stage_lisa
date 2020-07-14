#include <AccelStepper.h>
#include <IRremote.h>
//#include <IRutils.h>

#define RECV  2 //PORTD2
#define EN    3 //PORTD3
#define m0    4 //PORTD4
#define m1    5 //PORTD5
#define m2    6 //PORTD6
#define DIR   8 //PORTB0
#define STEP  7 //PORTD7
//#define IR1 20DF8877

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

decode_results resultsIR;
IRrecv irrecv(RECV);
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

  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");
}

void loop(){ 
  
  if(irrecv.decode(&resultsIR)){
    // String hex = hex2String(resultsIR.value) ; ////
    String hex = String(resultsIR.value) ;
    irrecv.resume(); // Receive the next value
    Serial.println(resultsIR.value);
    
    if(hex == "551520375"){
      turntableMotor.move(3200); 
       
      while(turntableMotor.distanceToGo()>0){
        Serial.println(turntableMotor.distanceToGo());
        turntableMotor.run();
      }
    }
    delay(1000);
  }

}
