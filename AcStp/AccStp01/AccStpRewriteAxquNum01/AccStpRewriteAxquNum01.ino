#include <AccelStepper.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <IRremote.h>

// ---- Pinnage Arduino nano ------
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

// ---- caractéristique moteur -----
const uint8_t ST_MOD = 4 ; // Stepping mode
const uint8_t RP = 100 ;   // Réducteur planétaire

// ---- Télécommande --------------
const uint32_t LG_NUM0 =  551487735 ;
const uint32_t LG_NUM1 =  551520375 ;
const uint32_t LG_NUM2 =  551504055 ;
const uint32_t LG_NUM3 =  551536695 ;
const uint32_t LG_NUM4 =  551495895 ;
const uint32_t LG_NUM5 =  551528535 ;
const uint32_t LG_NUM6 =  551512215 ;
const uint32_t LG_NUM7 =  551544855 ;
const uint32_t LG_NUM8 =  551491815 ;
const uint32_t LG_NUM9 =  551524455 ;

const uint32_t LG_RA    = 551486205 ; 
const uint32_t LG_DA    = 551518845 ;
const uint32_t LG_OFF   = 551489775 ;
const uint32_t LG_INPUT = 551538735 ;
const uint32_t LG_MENU  = 551535165 ;
const uint32_t LG_INFO  = 551507370 ;
const uint32_t LG_TEXT  = 551509665 ;
const uint32_t LG_GUIDE = 551540010 ;
const uint32_t LG_HOME  = 551501505 ;
const uint32_t LG_VIEW  = 551508135 ;
const uint32_t LG_BACK  = 551490795 ;
const uint32_t LG_EXIT  = 551541285 ;

//--- Variables globales -----------
bool enableMotor = LOW; 
uint32_t config_expTime = 1;
uint32_t config_interv =1;
uint32_t config_accel = 400;
uint32_t config_maxSpeed = 750;
uint8_t config_mode = 3;
// ---- Objets -------------------------

LiquidCrystal lcd(15, 14,16,17,18,19); // (rs,en,d4,d5,d6,d7)
decode_results resultsIR;
IRrecv irrecv(RECV);

//------  Nouvel classe Mstepper fille de la classe Accelstepper----------------------------------------------------
class MStepper: public AccelStepper
{ 
  public:
    MStepper():AccelStepper(AccelStepper::DRIVER,STEP,DIR) {}
//------------------------------ function initPos -----------------------------------------------------------    
    void initPos() {
      if(!speed())setSpeed(400);
      while(digitalRead(SHALL)) runSpeed(); 
      setCurrentPosition(0);
    }// end  function initPos
    

//------------------------------ function snap ---------------------------------------------------------------- 
  void snap (uint32_t expTime, uint8_t pinFocus, uint8_t pinTrig) {
    digitalWrite (pinFocus, LOW);
    delay (expTime*1000); // conversion en millisecondes
    digitalWrite (pinTrig, LOW);
    delay(100);  
    digitalWrite (pinTrig, HIGH);  
    digitalWrite (pinFocus, HIGH);
    return;    
  } // end function snapp
  
//------------------------------ function snap2 ---------------------------------------------------------------- 
  void snap2() {
    //global config_expTime;
    digitalWrite (FOCUS, LOW);
    delay (config_expTime*1000); // conversion en millisecondes
    digitalWrite (TRIG, LOW);
    delay(100);  
    digitalWrite (TRIG, HIGH);  
    digitalWrite (FOCUS, HIGH);
    return;    
  } // end function snapp


//------------------------------ function runSnap ----------------------------------------------------------- 
  void runSnap(uint32_t interval,uint32_t expTime, uint8_t pinFocus, uint8_t pinTrig, decode_results objIRdecode, IRrecv objIRrecv,LiquidCrystal objLCD){

    setCurrentPosition(0);
    while(currentPosition() < 200*ST_MOD*RP){ // = 360°
      if(objIRrecv.decode(&objIRdecode)){      
        if (objIRdecode.value == LG_EXIT) return; //bouton exit        
        objIRrecv.resume(); // Receive the next value   
      }
      objLCD.clear();
      objLCD.print("Progress : ");
      objLCD.print(currentPosition()/((200*ST_MOD*RP)/100)); 
      objLCD.print(" %"); 
      snap(expTime, pinFocus, pinTrig);
      move(angleToStep(interval%360));
      runToPosition();
    }
    objLCD.clear();
    objLCD.print("Progess: 100 %");
    delay(500);
    return; 
  } // end function RunSnapp

//------------------------------ function runSnap 2 ----------------------------------------------------------- 
  void runSnap2(uint32_t interval,uint32_t expTime, uint8_t pinFocus, uint8_t pinTrig, decode_results objIRdecode, IRrecv objIRrecv,LiquidCrystal objLCD){
    
    int i = 360/interval;
    float j =  i;
    setCurrentPosition(0);
    for(i;i>0;i--){ // = 360°
      if(objIRrecv.decode(&objIRdecode)){      
        if (objIRdecode.value == LG_EXIT) return; //bouton exit        
        objIRrecv.resume(); // Receive the next value   
      }
      Serial.println(i);
      Serial.println(currentPosition());
      Serial.println(stepToAngle(currentPosition()));
      Serial.print((j-i)/j*100);
      Serial.println("%");
      objLCD.clear();
      objLCD.print("Progress : ");
      objLCD.print((j-i)/j*100); 
      objLCD.print(" %"); 
      snap2();
      move(angleToStep(interval%360));
      runToPosition();
    }
    objLCD.clear();
    objLCD.print("Progess: 100 %");
    delay(500);
    return; 
  } // end function RunSnapp



//------------------------------ function stepToAngle ----------------------------------------------------------- 

float stepToAngle (long stepValue) { //configuré sur un mode micro step + reducteur 1:100  : (SV*AS)/(M*R)
  float angleValue = (stepValue%(200*ST_MOD*RP)) *(1.8 /(ST_MOD*RP));
  return (angleValue);
}// end function stepToAngle

//------------------------------ function angleToStep ----------------------------------------------------------- 

long angleToStep (float angleValue){ //configuré sur microstep + reducteur 1:100 : (AV*M*R)/ AS
  float stepValue = angleValue*(ST_MOD*RP)/1.8;
  return (stepValue);
} // end function angleToStep

//------------------------------ function set_mode ----------------------------------------------------------- 
uint8_t set_mode(uint8_t mode, uint8_t m0, uint8_t m1, uint8_t m2){
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

MStepper turntable;




void setup(){  
  Serial.begin(115200);
  pinMode(EN, OUTPUT);
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
  delay(1000);
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
    else if(resultsIR.value == LG_RA){ // Bouton flèche Haut
      turntable.move(turntable.angleToStep(1)); 
      turntable.run();  
    }
    else if(resultsIR.value == LG_DA){ // Bouton flèche Haut
      turntable.move(-turntable.angleToStep(1)); 
      turntable.run();   
    }
    else if(resultsIR.value == LG_INPUT){ // Bouton Imput
      lcd.clear();
      lcd.print("Time: ");
      lcd.print(config_expTime); 
      config_expTime = acqNum(3,config_expTime); 
    }
    else if(resultsIR.value == LG_MENU){ // Bouton Q.Menu
      lcd.clear();
      lcd.print("Accel : ");
      lcd.print(config_accel);   
      turntable.setAcceleration(config_accel=acqNum(3,config_accel)); 
    } 
    else if(resultsIR.value == LG_INFO){ // Bouton Info
      lcd.clear();
      lcd.print("Max Speed : ");
      lcd.print(config_maxSpeed);
      config_maxSpeed = acqNum(4,config_maxSpeed);  
      turntable.setMaxSpeed(config_maxSpeed); 
    }  
    else if(resultsIR.value == LG_HOME){ // Bouton Home
      lcd.clear();
      lcd.print("Interval ?");
      
      config_interv = acqNum(3,config_interv);
      lcd.clear();
      lcd.print("Temps d expo ?");
      config_expTime = acqNum(3,config_expTime);
  
      turntable.runSnap(config_interv,config_expTime,FOCUS,TRIG,resultsIR,irrecv,lcd);
    }
    else if(resultsIR.value == LG_NUM1){ // Bouton 1
      lcd.clear();
      digitalWrite (RLED, HIGH);
      lcd.print("Preset1: Run 10 ");
      delay(500);
      turntable.runSnap(10,config_expTime,FOCUS,TRIG,resultsIR,irrecv,lcd);
      digitalWrite (RLED, LOW);
    }
    else if(resultsIR.value == LG_NUM2){ // Bouton 2
      lcd.clear();
      digitalWrite (RLED, HIGH);
      lcd.print("Preset2: Run 15 ");
      delay(1000);
      turntable.runSnap(15,config_expTime,FOCUS,TRIG,resultsIR,irrecv,lcd);
      digitalWrite (RLED, LOW);
    }
    else if(resultsIR.value == LG_NUM3){ // Bouton 3
      lcd.clear();
      digitalWrite (RLED, HIGH);
      lcd.print("Preset3: Run 20");
      delay(1000);
      turntable.runSnap(20,config_expTime,FOCUS,TRIG,resultsIR,irrecv,lcd);
      digitalWrite (RLED, LOW);
    }
    else if(resultsIR.value == LG_NUM4){ // Bouton 4
      lcd.clear();
      digitalWrite (RLED, HIGH);
      lcd.print("Preset4: Run 30");
      delay(1000);
      turntable.runSnap(30,config_expTime,FOCUS,TRIG,resultsIR,irrecv,lcd);
      digitalWrite (RLED, LOW);
    }
    else if(resultsIR.value == LG_NUM5){ // Bouton 5
      lcd.clear();
      digitalWrite (RLED, HIGH);
      lcd.print("Preset5: Run 90");
      delay(1000);
      turntable.runSnap2(90,config_expTime,FOCUS,TRIG,resultsIR,irrecv,lcd);
      digitalWrite (RLED, LOW);
    }
    else if(resultsIR.value == LG_NUM0){ // Bouton 0
      lcd.clear();
      lcd.print("Mise a 0");
      turntable.setCurrentPosition(0);
      delay(1000);
    }    
    else if(resultsIR.value == LG_VIEW){ // Bouton Q.View
      digitalWrite (RLED, HIGH);
      turntable.initPos();
      digitalWrite (RLED, LOW);
    }  
    else if (resultsIR.value == LG_GUIDE){ // Bouton Guide.
      turntable.moveTo(turntable.angleToStep(float(acqNum(3,turntable.stepToAngle(turntable.currentPosition()))%360)));
      turntable.runToPosition();       
    } //end else if

    lcd.clear();
    lcd.print("Position: ");  
    lcd.print(turntable.stepToAngle(turntable.currentPosition()));
    delay(100);
  }// end if IRrecv
  digitalWrite (GLED, !enableMotor);

  
}// end Main loop







//------------------------------ function acqNum -----------------------------------------------------------
int acqNum(int numbChar,int ExitValue){ 
  
  int tab[numbChar];
  int numb =0;
  int i;
  digitalWrite (RLED, HIGH);
  for(i=0;i<numbChar;i++){
    tab[i] = 0xff ;
    lcd.setCursor(i,2);
    lcd.print(".");
  }
  delay(200); //anti rebond
  for(i=0;i<numbChar;i++){
    while(tab[i]==0xff){
      if(irrecv.decode(&resultsIR)){      
        if (resultsIR.value == LG_NUM1)      tab[i] = 1;
        else if (resultsIR.value == LG_NUM2) tab[i] = 2;
        else if (resultsIR.value == LG_NUM3) tab[i] = 3;
        else if (resultsIR.value == LG_NUM4) tab[i] = 4;
        else if (resultsIR.value == LG_NUM5) tab[i] = 5;
        else if (resultsIR.value == LG_NUM6) tab[i] = 6; 
        else if (resultsIR.value == LG_NUM7) tab[i] = 7;
        else if (resultsIR.value == LG_NUM8) tab[i] = 8;       
        else if (resultsIR.value == LG_NUM9) tab[i] = 9;        
        else if (resultsIR.value == LG_NUM0) tab[i] = 0; 
        else if (resultsIR.value == LG_EXIT) return(ExitValue); // Exit 
        else if (resultsIR.value == LG_BACK) {   // Back 
          i=max(i-1,0); //empêche l'indice d'être négtif en cas de spam
          tab[i]=0xff;
          lcd.setCursor(i,2);
          lcd.print(".");
        }        
        delay(100);       
        irrecv.resume(); // Receive the next value   
      }// end if
    }// end while
    lcd.setCursor(i,2);
    lcd.print(tab[i]);
  }// end for
  digitalWrite (RLED, LOW);
  for(i=0;i<numbChar;i++) numb = numb + tab[i]*pow(10,numbChar-(i+1));
  return (numb);
}// end function acqNum 
