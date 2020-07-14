#include <AccelStepper.h>
#include <LiquidCrystal.h>
#include <IRremote.h>

#define RECV  2   //PORTD2
#define EN    3   //PORTD3 
#define MODE0 4   //PORTD4
#define MODE1 5   //PORTD5
#define MODE2 6   //PORTD6
#define STEP  7   //PORTD7
#define DIR   8   //PORTB0
#define RLED  9   //PORTB1
#define GLED  10  //PORTB2
#define SHALL 11  //PORTB3
#define TRIG  12  //PORTD0 
#define FOCUS 13  //PORTD1

#define LG0  551487735 




//Nouvel classe Mstepper fille de la classe Accelstepper----------------------------------------------------
class MStepper: public AccelStepper
{ 
  public:
    MStepper():AccelStepper(AccelStepper::DRIVER,STEP,DIR) {}
//------------------------------ function initPos -----------------------------------------------------------    
    void initPos(int sensorHall) {
      if(!speed())setSpeed(800);
      while(digitalRead(sensorHall)) runSpeed(); 
      setCurrentPosition(0);
    }// end  function initPos
    
//------------------------------ function acqNum -----------------------------------------------------------
    int acqNum(int numbChar,decode_results objIRdecode, IRrecv objIRrecv,LiquidCrystal objLCD){ 
      
      int tab[3]= {0xff,0xff,0xff};
      int numb =0;
      int i;
      for(i=0;i<numbChar;i++){
        objLCD.setCursor(i,2);
        objLCD.print(".");
      }
      delay(200); //anti rebond
      for(i=0;i<numbChar;i++){
        while(tab[i]==0xff){
          if(objIRrecv.decode(&objIRdecode)){      
            if (objIRdecode.value == 551520375)      tab[i] = 1;
            else if (objIRdecode.value == 551504055) tab[i] = 2;
            else if (objIRdecode.value == 551536695) tab[i] = 3;
            else if (objIRdecode.value == 551495895) tab[i] = 4;
            else if (objIRdecode.value == 551528535) tab[i] = 5;
            else if (objIRdecode.value == 551512215) tab[i] = 6; 
            else if (objIRdecode.value == 551544855) tab[i] = 7;
            else if (objIRdecode.value == 551491815) tab[i] = 8;       
            else if (objIRdecode.value == 551524455) tab[i] = 9;        
            //else if (objIRdecode.value == 551487735) tab[i] = 0;
            else if (objIRdecode.value == LG0) tab[i] = 0; //test
            else if (objIRdecode.value == 551541285) return(stepToAngle(currentPosition())); // Exit 
            else if (objIRdecode.value == 551490795) {   // Back 
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
      for(i=0;i<numbChar;i++) numb = numb + tab[i]*pow(10,numbChar-(i+1));
      return (numb);
    }// end function acqNum 

//------------------------------ function snap ----------------------------------------------------------- 
  void snap (int expTime, int pinFocus, int pinTrig) {
    digitalWrite (pinFocus, LOW);
    delay (expTime*1000); // conversion en millisecondes
    digitalWrite (pinTrig, LOW);
    delay(200);  
    digitalWrite (pinTrig, HIGH);  
    digitalWrite (pinFocus, HIGH);
    return;    
  } // end function snapp

//------------------------------ function runSnap ----------------------------------------------------------- 
  void runSnap(int interval,int expTime, int pinFocus, int pinTrig, decode_results objIRdecode, IRrecv objIRrecv,LiquidCrystal objLCD){
      int mem = currentPosition();
      
      setCurrentPosition(0);
    while(currentPosition() < 6400){
      if(objIRrecv.decode(&objIRdecode)){      
        if (objIRdecode.value == "551541285") return; //bouton exit        
        objIRrecv.resume(); // Receive the next value   
        }
      objLCD.clear();
      /*
      objLCD.print("Progress : ");
      objLCD.print(currentPosition()/64);   /// A modifier
      objLCD.print(" %"); 
      */ 
      snap(expTime, pinFocus, pinTrig);
      move(angleToStep(interval));
      runToPosition();
    }
    objLCD.clear();
    objLCD.print("Progess: 100 %");
    initPos(SHALL);
    return; 
  } // end function RunSnapp


//------------------------------ function stepToAngle ----------------------------------------------------------- 

float stepToAngle (int stepValue) { //configuré sur un mode micro step + reducteur 1:100  : (SV*AS)/(M*R)
  float angleValue = (stepValue%(200*100)) *(1.8 /(1*100));
  return (angleValue);
}// end function stepToAngle

//------------------------------ function angleToStep ----------------------------------------------------------- 

int angleToStep (float angleValue){ //configuré sur microstep + reducteur 1:100 : (AV*M*R)/ AS
  float stepValue = angleValue*(1*100)/1.8;
  return (stepValue);
} // end function angleToStep

//------------------------------ function set_mode ----------------------------------------------------------- 
int set_mode(int mode, int m0, int m1, int m2){
  switch (mode){
    case 1: //full step
      digitalWrite(m0,LOW);
      digitalWrite(m1,LOW);
      digitalWrite(m2,LOW);
      return(1); 
   case 2: // 1/2 step
      digitalWrite(m0,HIGH);
      digitalWrite(m1,LOW);
      digitalWrite(m2,LOW);
      return(2); 
  case 3: //  1/4 step
      digitalWrite(m0,LOW);
      digitalWrite(m1,HIGH);
      digitalWrite(m2,LOW);
      return(4);  
  case 4: //  1/8 step
      digitalWrite(m0,HIGH);
      digitalWrite(m1,HIGH);
      digitalWrite(m2,LOW);
      return(8); 
  case 5: //  1/16 step
      digitalWrite(m0,LOW);
      digitalWrite(m1,LOW);
      digitalWrite(m2,HIGH);
      return(16); 
  case 6: //  1/32 step
      digitalWrite(m0,HIGH);
      digitalWrite(m1,HIGH);
      digitalWrite(m2,HIGH);
      return(32);  
  default: //  1 step
      digitalWrite(m0,LOW);
      digitalWrite(m1,LOW);
      digitalWrite(m2,LOW);
      return(1);        
  }// end switch  
} //end function set_mode
};
///-----------------------------------------------------------------------------------------------

decode_results resultsIR;
IRrecv irrecv(RECV);
MStepper turntable;
LiquidCrystal lcd(15, 14,16,17,18,19); // (rs,en,d4,d5,d6,d7)

const uint64_t LG_HA = 551486205 ;
const uint64_t LG_OFF = 551489775 ;
const uint64_t LG_INPUT = 551538735 ;
const uint64_t LG_MENU = 551535165 ;
const uint64_t LG_INFO = 551507370 ;
const uint64_t LG_TEXT = 551509665 ;
const uint64_t LG_GUIDE = 551540010 ;
const uint64_t LG_HOME = 551501505 ;
const uint64_t LG_VIEW = 551508135 ;




bool enableMotor = LOW; 
int config_expTime = 1;
int config_interv =1;
int config_accel = 800;
int config_maxSpeed = 999;
int config_mode = 1;


void setup(){  
  Serial.begin(115200);
  pinMode (EN, OUTPUT);
  pinMode( MODE0 , OUTPUT );
  pinMode( MODE1 , OUTPUT );
  pinMode( MODE2 , OUTPUT );
  pinMode( GLED , OUTPUT );
  pinMode( RLED , OUTPUT );
  pinMode( FOCUS, OUTPUT );
  pinMode( TRIG , OUTPUT );
  pinMode (SHALL , INPUT);
  digitalWrite(EN, LOW);
  digitalWrite(FOCUS, HIGH);
  digitalWrite(TRIG , HIGH);
  turntable.set_mode(config_mode,MODE0,MODE1,MODE2);
  turntable.setMaxSpeed(config_maxSpeed);
  turntable.setAcceleration(config_accel); 
  lcd.begin(16, 2);
  lcd.print("Initializes");
  irrecv.enableIRIn(); // Start the receiver

  // turntable.initPos(SHALL); //desactivate
  lcd.clear();
  lcd.print("Position: ");  
  lcd.print(turntable.stepToAngle(turntable.currentPosition()));  
}

void loop(){ 
  
  if(irrecv.decode(&resultsIR)){
    irrecv.resume(); // Receive the next value
   
    if (resultsIR.value == LG_OFF){ //Bouton off
      enableMotor = !enableMotor ;
      digitalWrite(EN,enableMotor);
    }
    
    else if(resultsIR.value == LG_HA){ // Bouton flèche Haut
      turntable.move(10000); //1/2 de tour     
      turntable.runToPosition();
    }
    
    else if(resultsIR.value == LG_INPUT){ // Bouton Imput
      lcd.clear();
      lcd.print("Speed : ");
      lcd.print(turntable.speed());
      turntable.setSpeed(turntable.acqNum(3,resultsIR,irrecv,lcd));
      
    }
    else if(resultsIR.value == LG_MENU){ // Bouton Q.Menu
      lcd.clear();
      lcd.print("Accel : ");
      lcd.print(config_accel);   
      turntable.setAcceleration(config_accel=turntable.acqNum(3,resultsIR,irrecv,lcd)); 
    } 
    
    else if(resultsIR.value == LG_INFO){ // Bouton Info
      lcd.clear();
      lcd.print("Mode : ");
      lcd.print(config_mode);
      config_mode= turntable.set_mode(turntable.acqNum(1,resultsIR,irrecv,lcd),MODE0,MODE1,MODE2);    
    }
    
    else if(resultsIR.value == LG_TEXT){ // Bouton Text
      lcd.clear();
      lcd.print("Max Speed : ");
      lcd.print(config_maxSpeed);
      config_maxSpeed = turntable.acqNum(4,resultsIR,irrecv,lcd);  
      turntable.setMaxSpeed(config_maxSpeed); 
    }
          
    else if(resultsIR.value == LG_HOME){ // Bouton Home
      lcd.clear();
      lcd.print("Interval ?");
      digitalWrite (RLED, HIGH);
      config_interv = turntable.acqNum(3,resultsIR,irrecv,lcd);
      lcd.clear();
      lcd.print("Temps d expo ?");
      config_expTime = turntable.acqNum(3,resultsIR,irrecv,lcd);
      digitalWrite (RLED, LOW);  
      turntable.runSnap(config_interv,config_expTime,FOCUS,TRIG,resultsIR,irrecv,lcd);
    }

    else if(resultsIR.value == LG_VIEW){ // Bouton Q.View
      digitalWrite (RLED, HIGH);
      turntable.initPos(SHALL);
      digitalWrite (RLED, LOW);
    }
   
    else if (resultsIR.value == LG_GUIDE){ // Bouton Guide.
      digitalWrite (RLED, HIGH);
      turntable.moveTo(turntable.angleToStep(float(turntable.acqNum(3,resultsIR,irrecv,lcd)%360)));
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
