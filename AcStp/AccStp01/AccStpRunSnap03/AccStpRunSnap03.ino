#include <AccelStepper.h>
#include <LiquidCrystal.h>
#include <IRremote.h>

#define RECV  2   //PORTD2
#define EN    3   //PORTD3 
#define MOD0  4   //PORTD4
#define MOD1  5   //PORTD5
#define MOD2  6   //PORTD6
#define STEP  7   //PORTD7
#define DIR   8   //PORTB0
#define RLED  9   //PORTB1
#define GLED  10  //PORTB2
#define SHALL 11  //PORTB3
#define TRIG  12  //PORTD0 
#define FOCUS 13  //PORTD1


//Nouvel classe Mstepper fille de la classe Accelstepper----------------------------------------------------
class MStepper: public AccelStepper
{ 
  public:
    MStepper():AccelStepper(AccelStepper::DRIVER,STEP,DIR) {}
//------------------------------ Fonction initPos -----------------------------------------------------------    
    void initPos(int sensorHall) {
      setSpeed(800);
      while(digitalRead(sensorHall)) runSpeed(); 
      setCurrentPosition(0);
    }// end  initPos
    
//------------------------------ fonction acqNum -----------------------------------------------------------
    int acqNum(decode_results objIRdecode, IRrecv objIRrecv,LiquidCrystal objLCD){ 
      
      int tab[3]= {0xff,0xff,0xff};
      int numb =0;
      int i;
      String codeIR;
   
      objLCD.setCursor(0,2);
      objLCD.print("...");
      delay(200); //anti rebond
      for(i=0;i<3;i++){
        while(tab[i]==0xff){
          if(objIRrecv.decode(&objIRdecode)){      
            codeIR =objIRdecode.value;
            if (codeIR == "551520375")      tab[i] = 1;
            else if (codeIR == "551504055") tab[i] = 2;
            else if (codeIR == "551536695") tab[i] = 3;
            else if (codeIR == "551495895") tab[i] = 4;
            else if (codeIR == "551528535") tab[i] = 5;
            else if (codeIR == "551512215") tab[i] = 6; 
            else if (codeIR == "551544855") tab[i] = 7;
            else if (codeIR == "551491815") tab[i] = 8;       
            else if (codeIR == "551524455") tab[i] = 9;        
            else if (codeIR == "551487735") tab[i] = 0;
            else if (codeIR == "551541285") return(stepToAngle(currentPosition())); // Exit 
            else if (codeIR == "551490795") {   // Back 
              i=max(i-1,0); //empêche l'indice d'être négtif en cas de spam
              tab[i]=0xff;
              objLCD.setCursor(i,2);
              objLCD.print(".");
            }        
            delay(100);       
            objIRrecv.resume(); // Receive the next value   
          }// end if
        }// end while
        objLCD.setCursor(i,2);
        objLCD.print(tab[i]);
      }// end for
      return (numb =(tab[0]*100 + tab[1]*10 + tab[2])); 
    }// end acqNum 

//------------------------------ fonction snap ----------------------------------------------------------- 
  void snap (int expTime, int pinFocus, int pinTrig) {
    Serial.println("Hello");
    digitalWrite (pinFocus, LOW);
    delay (expTime*1000); // conversion en millisecondes
    digitalWrite (pinTrig, LOW);
    Serial.println("Triggered");
    delay(200);  
    digitalWrite (pinTrig, HIGH);  
    digitalWrite (pinFocus, HIGH);
    return;    
  }

//------------------------------ fonction runSnap ----------------------------------------------------------- 
  void runSnap(int interval,int expTime, int pinFocus, int pinTrig, decode_results objIRdecode, IRrecv objIRrecv,LiquidCrystal objLCD){
      int mem = currentPosition();
      setCurrentPosition(0);
    while(currentPosition() < 6400){
      objLCD.clear();
      objLCD.print(" progress : ");
      objLCD.print(currentPosition()/64);   
      objLCD.print(" %");  
      snap(expTime, pinFocus, pinTrig);
      move(angleToStep(interval));
      runToPosition();
    }

    return;
      
      
  }


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
int config_expTime = 1;
int config_interv=1;
int returnValue;




void setup(){  
  Serial.begin(115200);
  pinMode (EN, OUTPUT);
  pinMode( MOD0 , OUTPUT );
  pinMode( MOD1 , OUTPUT );
  pinMode( MOD2 , OUTPUT );
  pinMode( GLED , OUTPUT );
  pinMode( RLED , OUTPUT );
  pinMode( FOCUS, OUTPUT );
  pinMode( TRIG , OUTPUT );
  pinMode (SHALL , INPUT);
  digitalWrite(EN, LOW);
  digitalWrite(MOD0 , HIGH);
  digitalWrite(MOD1 , HIGH);
  digitalWrite(MOD2 , HIGH);
  digitalWrite(FOCUS, HIGH);
  digitalWrite(TRIG , HIGH);
  
  turntable.setMaxSpeed(1800);
  turntable.setAcceleration(800); 
  lcd.begin(16, 2);
  lcd.print("Initializes");
  irrecv.enableIRIn(); // Start the receiver

  //turntable.initPos(SHALL);
  lcd.clear();
  lcd.print("Position: ");  
  lcd.print(turntable.stepToAngle(turntable.currentPosition()));  
}

void loop(){ 
  
  if(irrecv.decode(&resultsIR)){
    String codeIR = String(resultsIR.value) ;
    Serial.println(codeIR);
    irrecv.resume(); // Receive the next value
   
    
    if (codeIR == "551489775"){ //Bouton off
      enableMotor = !enableMotor ;
      digitalWrite(EN,enableMotor);
      }
    else if(codeIR == "551486205"){ // Bouton flèche Haut
      turntable.move(1600); //1/4 de tour     
      turntable.runToPosition();
    }

    else if(codeIR == "551518845"){ // Bouton flèche Bas
      lcd.clear();
      lcd.print("Temps d expo ?");
      config_expTime = turntable.acqNum(resultsIR,irrecv,lcd);  
      turntable.snap(config_expTime,FOCUS,TRIG);
    
    }

    else if(codeIR == "551542815"){ // Bouton flèche Gauche inutile !!!!
      digitalWrite(FOCUS, LOW);
      digitalWrite(TRIG, LOW);
      delay (3000);
      digitalWrite(FOCUS, HIGH);
      digitalWrite(TRIG, HIGH);

    }

    else if(codeIR == "551489010"){ // Bouton play
      lcd.clear();
      lcd.print("interval ?");
      digitalWrite (RLED, HIGH);
      config_interv = turntable.acqNum(resultsIR,irrecv,lcd);
      lcd.clear();
      lcd.print("Temps d expo ?");
      config_expTime = turntable.acqNum(resultsIR,irrecv,lcd);
      digitalWrite (RLED, LOW);  
      turntable.runSnap(config_interv,config_expTime,FOCUS,TRIG,resultsIR,irrecv,lcd );
    }



    else if(codeIR == "551508135"){ // Bouton Q.View
      digitalWrite (RLED, HIGH);
      turntable.initPos(SHALL);
      digitalWrite (RLED, LOW);
    }
   
    else if (codeIR == "551540010"){ // Bouton Guide.
      digitalWrite (RLED, HIGH);
      turntable.moveTo(turntable.angleToStep(float(turntable.acqNum(resultsIR,irrecv,lcd)%360)));
      digitalWrite (RLED, LOW);
      turntable.runToPosition();
        
    }

    lcd.clear();
    lcd.print("Position: ");  
    lcd.print(turntable.stepToAngle(turntable.currentPosition()));
    delay(100);
  }// end if IRrecv
  digitalWrite (GLED, !enableMotor);
}// end Main loop
