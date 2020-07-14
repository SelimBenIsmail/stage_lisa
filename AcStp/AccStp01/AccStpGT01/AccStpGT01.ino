#include <AccelStepper.h>
#include <IRremote.h>


#define RECV  2 //PORTD2
#define EN    3 //PORTD3 // intérêt ?
#define m0    4 //PORTD4
#define m1    5 //PORTD5
#define m2    6 //PORTD6
#define DIR   8 //PORTB0
#define STEP  7 //PORTD7
#define RLED  16
#define GLED  15

class MStepper: public AccelStepper
{ 
  public:
    MStepper():AccelStepper(AccelStepper::DRIVER,STEP,DIR) {}
    
    void stop1() {
      if (_direction == DIRECTION_CW) moveTo(currentPosition() + 1600);
      else moveTo(currentPosition() - 1600);
    }// end stop1

    int aquiNum(uint8_t led, decode_results objIRdecode, IRrecv objIRrecv){ 
      
      int tab[3]= {0xff,0xff,0xff};
      int angle =0;
      int i;
      String codeIR;
      digitalWrite (led, HIGH);
      Serial.println("Hello");
      
      for(i=0;i<3;i++){
        while(tab[i]==0xff){
          if(objIRrecv.decode(&objIRdecode)){  
            if (codeIR = (objIRdecode.value)== "551520375")
                tab[i] = 1;
            else if (codeIR = (objIRdecode.value)== "551504055")
                tab[i] = 2;
            else if (codeIR = (objIRdecode.value)== "551536695")
                tab[i] = 3;
            else if (codeIR = (objIRdecode.value)== "551495895")
                tab[i] = 4;
            else if (codeIR = (objIRdecode.value)== "551528535")
                tab[i] = 5;
            else if (codeIR = (objIRdecode.value)== "551512215")
                tab[i] = 6;
            else if (codeIR = (objIRdecode.value)== "551544855")
                tab[i] = 7;
            else if (codeIR = (objIRdecode.value)== "551491815")
                tab[i] = 8;
            else if (codeIR = (objIRdecode.value)== "551524455")
                tab[i] = 9;
            else if (codeIR = (objIRdecode.value)== "551487735")
                tab[i] = 0;
           delay(1000);       
           objIRrecv.resume(); // Receive the next value
           
          }// end if
        }// end while
        Serial.print("tab: ");
        Serial.print(i);
        Serial.println(" check !");
        Serial.print("valeur :");
        Serial.println(tab[i]);
        Serial.print("Ir :");
        Serial.println(objIRdecode.value);
        
      }// end for
      digitalWrite (led, LOW);
      angle = tab[0]*100 + tab[1]*10 + tab[2];
      Serial.print("angle: ");
      Serial.println(angle);
      Serial.println("bye");
    }// end aquiNum 

   
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

bool  enableMotor = HIGH; ///

void setup(){  
  Serial.begin(115200);
  pinMode (EN, OUTPUT);
  pinMode( m0 , OUTPUT );
  pinMode( m1 , OUTPUT );
  pinMode( m2 , OUTPUT );
  pinMode( GLED , OUTPUT );
  digitalWrite(EN, LOW);
  digitalWrite(m0 , HIGH);
  digitalWrite(m1 , HIGH);
  digitalWrite(m2 , HIGH);

  turntableMotor.setMaxSpeed(1800);
  turntableMotor.setAcceleration(800); 

  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");
  digitalWrite (GLED, HIGH);

}

void loop(){ 
  
  if(irrecv.decode(&resultsIR)){
    String codeIR = String(resultsIR.value) ;
    irrecv.resume(); // Receive the next value
    Serial.println(resultsIR.value);
    
    if (codeIR == "551489775"){ //Bouton off
      enableMotor = !enableMotor ;
      digitalWrite(EN,enableMotor);
      }
    else if(codeIR == "551485695"){ // Bouton Prog +
      turntableMotor.move(1600); //1/4 de tour     
      while(turntableMotor.distanceToGo()>0)
        turntableMotor.run();
    }
    else if (codeIR == "551540010"){
       Serial.println("entrez le position à atteindre en degré, appuyer sur OK pour valider");
       turntableMotor.aquiNum(RLED,resultsIR,irrecv);
      
    }




    
    digitalWrite (GLED, !enableMotor);
    delay(1000);
  }

}
