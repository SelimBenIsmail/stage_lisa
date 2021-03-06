#include <AccelStepper.h>
#include <LiquidCrystal.h>
#include <IRremote.h>

#define RECV  2   //PORTD2
#define EN    3   //PORTD3 
#define MOD0  4   //PORTD4
#define MOD1  5   //PORTD5
#define MOD2  6   //PORTD6
#define DIR   8   //PORTB0
#define STEP  7   //PORTD7
#define RLED  9   //PORTB1
#define GLED  10  //PORTB2
#define SHALL 11  //PORTB3



//Nouvel classe Mstepper fille de la classe Accelstepper----------------------------------------------------
class MStepper: public AccelStepper
{ 
  public:
    MStepper():AccelStepper(AccelStepper::DRIVER,STEP,DIR) {}
//------------------------------ Fonction initPos -----------------------------------------------------------    
    void initPos(int sensorHall) {
      Serial.println("Hello"); 
      while(sensorHall){
        move(3200);
        while(distanceToGo()>0){
          if (digitalRead(sensorHall) == LOW){
            Serial.println("Sensor"); 
            return;
          }
          else run();
        }
        Serial.println("Loop"); 
      }
      return;
      
    }// end  initPos
    
//------------------------------ fonction AquiNum -----------------------------------------------------------
    int aquiNum(uint8_t led, decode_results objIRdecode, IRrecv objIRrecv,LiquidCrystal objLCD){ 
      
      int tab[3]= {0xff,0xff,0xff};
      int angle =0;
      int i;
      String codeIR;
      pinMode(led,OUTPUT);
      digitalWrite (led, HIGH);
      objLCD.setCursor(0,2);
      objLCD.print("...");
      delay(200); //anti rebond
      for(i=0;i<3;i++){
        while(tab[i]==0xff){
          if(objIRrecv.decode(&objIRdecode)){      
            codeIR =objIRdecode.value;
            if (codeIR == "551520375") //switchcase ne fonctionnent qu'avec des int ou des char, pas avec des strings
                tab[i] = 1;
            else if (codeIR == "551504055")
                tab[i] = 2;
            else if (codeIR == "551536695")
                tab[i] = 3;
            else if (codeIR == "551495895")
                tab[i] = 4;
            else if (codeIR == "551528535")
                tab[i] = 5;
            else if (codeIR == "551512215")
                tab[i] = 6;
            else if (codeIR == "551544855")
                tab[i] = 7;
            else if (codeIR == "551491815")
                tab[i] = 8;
            else if (codeIR == "551524455")
                tab[i] = 9;
            else if (codeIR == "551487735")
                tab[i] = 0;
           delay(100);       
           objIRrecv.resume(); // Receive the next value   
          }// end if
        }// end while
        Serial.print(tab[i]);
        objLCD.setCursor(i,2);
        objLCD.print(tab[i]);
      }// end for
      digitalWrite (led, LOW);
      angle = (tab[0]*100 + tab[1]*10 + tab[2])% 360;
      if (!angle) angle = 360;
      Serial.println("");
      Serial.print("Angle: ");
      Serial.println(angle);
      return (angleToStep(float(angle)));
      
    }// end aquiNum 
//------------------------------ fonction stepToAngle ----------------------------------------------------------- 

float stepToAngle (int stepValue) { //configuré sur un mode micro step: 1,8°/32
  float angleValue = (stepValue %  6400) /17.777;
  return (angleValue);
}

//------------------------------ fonction angleToStep ----------------------------------------------------------- 

int angleToStep (float angleValue){ //configuré sur un mode micro step: 1,8°/32
  float stepValue = angleValue * 17.777;
  return (stepValue);
}

 
  
//-------------------------------------------------------------------------------------------------------- 
  /*protected:
    void step(long step) {
      if (_direction) PORTB = PORTB | 0x01;
      else PORTB = PORTB & 0xFE;
      PORTD = PORTD | 0x80;
      delayMicroseconds(2);
      PORTD = PORTD & 0x7F;
    }
*/
};

decode_results resultsIR;
IRrecv irrecv(RECV);
MStepper turntable;
LiquidCrystal lcd(15, 14,16,17,18,19); // (rs,en,d4,d5,d6,d7)

bool  enableMotor = LOW; 


void setup(){  
  Serial.begin(115200);
  pinMode (EN, OUTPUT);
  pinMode( MOD0 , OUTPUT );
  pinMode( MOD1 , OUTPUT );
  pinMode( MOD2 , OUTPUT );
  pinMode( GLED , OUTPUT );
  pinMode( RLED , OUTPUT );
  pinMode (SHALL , INPUT);
  digitalWrite(EN, LOW);
  digitalWrite(MOD0 , HIGH);
  digitalWrite(MOD1 , HIGH);
  digitalWrite(MOD2 , HIGH);
  
  turntable.setMaxSpeed(1800);
  turntable.setAcceleration(800); 
  lcd.begin(16, 2);
  irrecv.enableIRIn(); // Start the receiver
  

}

void loop(){ 
  
  if(irrecv.decode(&resultsIR)){
    String codeIR = String(resultsIR.value) ;
    irrecv.resume(); // Receive the next value
   
    
    if (codeIR == "551489775"){ //Bouton off
      enableMotor = !enableMotor ;
      digitalWrite(EN,enableMotor);
      }
    else if(codeIR == "551486205"){ // Bouton flèche Haut
      turntable.move(1600); //1/4 de tour     
      turntable.runToPosition();
    }

    else if(codeIR == "551518845"){ // Bouton flèche bas
      digitalWrite (RLED, HIGH);
      turntable.initPos(SHALL);
      digitalWrite (RLED, LOW);
    }

     else if(codeIR == "551510175"){ // Bouton flèche droite
      while(1){
        if (SHALL == LOW){
          digitalWrite (RLED, HIGH);  
        }         
      }   
    }
   
    else if (codeIR == "551540010"){ // Bouton Guide.
      Serial.println("entrez le position à atteindre en degré");
      turntable.moveTo (turntable.aquiNum(RLED,resultsIR,irrecv,lcd));
      turntable.runToPosition();  
    }

    lcd.clear();
    lcd.print("Position: ");  
    lcd.print(turntable.stepToAngle(turntable.currentPosition()));
    delay(500);
  }// end if IRrecv
  digitalWrite (GLED, !enableMotor);
}// end Main loop
