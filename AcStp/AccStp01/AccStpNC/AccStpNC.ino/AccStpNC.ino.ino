#include <AccelStepper.h>
#include <LiquidCrystal.h>
#include <IRremote.h>

#define RECV  2 //PORTD2
#define EN    3 //PORTD3 
#define m0    4 //PORTD4
#define m1    5 //PORTD5
#define m2    6 //PORTD6
#define DIR   8 //PORTB0
#define STEP  7 //PORTD7
#define RLED  9
#define GLED  10



//Nouvel classe Mstepper fille de la classe Accelstepper----------------------------------------------------
class MStepper: public AccelStepper
{ 
  public:
    MStepper():AccelStepper(AccelStepper::DRIVER,STEP,DIR) {}
    
//------------------------------ fonction AquiNum -----------------------------------------------------------
    int aquiNum(uint8_t led, decode_results objIRdecode, IRrecv objIRrecv,LiquidCrystal objLCD){ 
      
      int tab[3]= {0xff,0xff,0xff};
      int angle =0;
      int i;
      String codeIR;
      
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
      angle = (tab[0]*100 + tab[1]*10 + tab[2])% 360;
      if (!angle) angle = 360;
      Serial.println("");
      Serial.print("Angle: ");
      Serial.println(angle);
      return (angleToStep(float(angle)));
      
    }// end aquiNum 
//------------------------------ fonction stepToAngle ----------------------------------------------------------- 

float stepToAngle (int stepValue) { //configuré sur un mode micro step: 1,8°/32
  float angle = (stepValue %  6400) /17.777;
  return (angle);
}

//------------------------------ fonction angleToStep ----------------------------------------------------------- 

int angleToStep (float angleValue){ //configuré sur un mode micro step: 1,8°/32
  float step = angleValue * 17.777;
  return (step);
}

//------------------------------ fonction vitesse + ----------------------------------------------------------- 
/*
int angleToStep (float angleValue){ //configuré sur un mode micro step: 1,8°/32
  float step = angleValue * 17.777;
  return (step);
}*/
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
  pinMode( m0 , OUTPUT );
  pinMode( m1 , OUTPUT );
  pinMode( m2 , OUTPUT );
  pinMode( GLED , OUTPUT );
  pinMode( RLED , OUTPUT );
  digitalWrite(EN, LOW);
  digitalWrite(m0 , HIGH);
  digitalWrite(m1 , HIGH);
  digitalWrite(m2 , HIGH);

  turntable.setMaxSpeed(1800);
  turntable.setAcceleration(800); 

  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");

  lcd.begin(16, 2);
  lcd.print("hello world!");
  delay(500);
  lcd.clear();

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
    else if(codeIR == "551486205"){ // Bouton flèche Haut
      turntable.move(1600); //1/4 de tour     
      turntable.runToPosition();
    }
    /*
    else if(codeIR == "551485695"){ //prog +
      delay(500);
      while(codeIR == "4294967295"){
        turntable.runSpeed();
      }
      */
    }
    else if (codeIR == "551540010"){ // Bouton Guide.
      Serial.println("entrez le position à atteindre en degré");
      digitalWrite (led, HIGH);
      turntable.moveTo (turntable.aquiNum(RLED,resultsIR,irrecv,lcd));
      digitalWrite (led, LOW);
      turntable.runToPosition();  
    }

    lcd.clear();
    lcd.print("Position: ");  
    lcd.print(turntable.stepToAngle(turntable.currentPosition()));



    
    
    delay(500);
  }// end if IRrecv
  digitalWrite (GLED, !enableMotor);
}// end Main loop
