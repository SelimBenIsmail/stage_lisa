#include <AccelStepper.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <IRremote.h>
#include <TimerOne.h>

// ---- Connexion Pin Arduino nano -------
#define RECV  2   //PORTD2
#define EN    3   //PORTD3 
#define M0    4   //PORTD4
#define M1    5   //PORTD5
#define M2    6   //PORTD6
#define STEP  7   //PORTD7
#define DIR   8   //PORTB0
#define RLED  9   //PORTB1
#define GLED  10  //PORTB2
#define SHALL 11  //PORTB3
#define TRIG  12  //PORTD0 
#define FOCUS A6  

// ---- caractéristique moteur -----------
const uint8_t ST_MOD = 4 ; // Stepping mode
const uint8_t RP = 100 ;   // Réducteur planétaire
const uint8_t MODE = 3;   // Sélection du mode dasns le switch case

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

//--- Adresse EEPROM --------------
const uint16_t ADRESS_CONF = 0x00 ;

//--- Variables globales -----------
bool enableMotor = LOW;
uint8_t state = 0; //Machine d'état
uint8_t lastState = 0 ; // Etat précédent
struct configuration{ 
  bool mem ; // false si une configuration est restée en mémoire 
  uint32_t expTime ; // temps d'attente avant le déclanchement de la camera
  uint32_t interv ; // distance angulaire entre chaque étape
  uint32_t accel ; // accelaration
  uint32_t max_Speed; // vitesse maximale
};
configuration conf = {1,1,1,400,750};

// --------- Objets ----------------------
LiquidCrystal lcd(15, 14,16,17,18,19); // (rs,en,d4,d5,d6,d7)
decode_results resultsIR;
IRrecv irrecv(RECV);

//-----------------------------------------Nouvel classe Mstepper fille de la classe Accelstepper -----------------------------
class MStepper: public AccelStepper
{ 
  public:
    MStepper():AccelStepper(AccelStepper::DRIVER,STEP,DIR) {}
    
  //---------- function initPos --------    
    void initPos() {
      lastState = state;
      state=8;
      if(!speed())setSpeed(400);
      while(digitalRead(SHALL)) runSpeed(); 
      setCurrentPosition(0);
    }// end  function initPos    
  
  //----------- function snap ---------------
  void snap() {
    lastState = state;
    state=7;
    digitalWrite (FOCUS, LOW);
    delay (conf.expTime*1000); // conversion en millisecondes
    digitalWrite (TRIG, LOW);
    delay(100);  
    digitalWrite (TRIG, HIGH);  
    digitalWrite (FOCUS, HIGH);
    return;    
  } // end function snapp


  //------------ function runSnap ------------------
  void runSnap(uint32_t interval){
    lastState = state;
    state = 6;
    long memPos=currentPosition();
    setCurrentPosition(0);
    while(currentPosition() < 200*ST_MOD*RP){ //80k steps= 360°
      lcd.clear();
      lcd.print("Progress : ");
      lcd.print(currentPosition()/((200*ST_MOD*RP)/100)); 
      lcd.print(" %"); 
      snap();
      move(angleToStep(interval%360));
      runToPosition();
    }
    lcd.clear();
    lcd.print("Progess: 100 %");
    setCurrentPosition(memPos);
    delay(500);
    return; 
  } // end function RunSnapp
/*
  //-------------- function runSnap 2 --------------------------
  void runSnap2(uint32_t interval){ 
    lastState = state;
    state = 6;  
    int i = 360/interval;
    float j =  i;
    setCurrentPosition(0);
    for(i;i>0;i--){ // = 360°
      if(irrecv.decode(&resultsIR)){      
        if (resultsIR.value == LG_EXIT) return; //bouton exit        
        irrecv.resume(); // Receive the next value  
      }
      Serial.println(i);
      Serial.println(currentPosition());
      Serial.println(stepToAngle(currentPosition()));
      Serial.print((j-i)/j*100);
      Serial.println("%");
      lcd.clear();
      lcd.print("Progress : ");
      lcd.print((j-i)/j*100); 
      lcd.print(" %"); 
      snap();
      move(angleToStep(interval%360));
      runToPosition();
    }
    lcd.clear();
    lcd.print("Progess: 100 %");
    delay(500);  
    return; 
  } // end function RunSnapp
*/

  //------------ function goTo ------------------
  void goTo(uint32_t destination){
    lastState = state;
    state=5;
    if(destination==0xfff)return ;
    moveTo(angleToStep(destination%360));
    runToPosition();     
    return; 
  } // end function goTo

  //-------- function stepToAngle ----------------------------------------------------------- 
  float stepToAngle (long stepValue) { //configuré sur un quart step + reducteur 1:100  
    float angleValue = (stepValue%(200*ST_MOD*RP)) *(1.8 /(ST_MOD*RP));
    return (angleValue);
  }// end function stepToAngle

  //------------------------------ function angleToStep ----------------------------------------------------------- 
  long angleToStep (float angleValue){ //configuré sur quart step + reducteur 1:100 
    float stepValue = angleValue*(ST_MOD*RP)/1.8;
    return (stepValue);
  }// end function angleToStep

  //------------------------------ function set_mode -------------------------------------------------------------- 
  uint8_t set_mode(uint8_t mode){
    lastState = state;
    state=10;
    switch (mode){
      case 1: //full step
        PORTD = PORTD & 0b10001111;
        return(1); 
     case 2: // 1/2 step
        PORTD = PORTD & 0b10001111;
        PORTD = PORTD | 0b00010000;
        return(2); 
    case 3: //  1/4 step
        PORTD = PORTD & 0b10001111;
        PORTD = PORTD | 0b00100000;
        return(4);  
    case 4: //  1/8 step
        PORTD = PORTD & 0b10001111;
        PORTD = PORTD | 0b01100000;
        return(8); 
    case 5: //  1/16 step
        PORTD = PORTD & 0b10001111;
        PORTD = PORTD | 0b01000000;
        return(16); 
    case 6: //  1/32 step
        PORTD = PORTD | 0b01110000;
        return(32);  
    default: //  1 step
        PORTD = PORTD & 0b10001111;
        return(1);        
    }// end switch  
  } //end function set_mode
};
MStepper turntable; 

void setup(){
  lastState = 0;
  state=0;  
  Serial.begin(115200);
  pinMode(EN, OUTPUT);
  pinMode( M0 , OUTPUT );
  pinMode( M1 , OUTPUT );
  pinMode( M2 , OUTPUT );
  pinMode( GLED , OUTPUT );
  pinMode( RLED , OUTPUT );
  pinMode( FOCUS, OUTPUT );
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode( TRIG , OUTPUT );
  pinMode (SHALL , INPUT);
  digitalWrite(EN, LOW);
  digitalWrite(RLED , LOW );
  digitalWrite(FOCUS, HIGH);
  digitalWrite(TRIG , HIGH);
  //--------------------------- Init LCD -------------------------------------------- 
  lcd.begin(16, 2);
  lcd.print("Initializes");
  irrecv.enableIRIn(); // Start the receiver
  delay(500);
  //--------------------------- Init Config ------------------------------------------
  turntable.set_mode(MODE);
  turntable.setMaxSpeed(conf.max_Speed);
  turntable.setAcceleration(conf.accel);
  loadEEPROM();
  // --------------------------- Init Timer -------------------------------------------
 // Timer1.initialize(500000);         // initialize timer1, and set a 1/2 second period
 // Timer1.attachInterrupt(Tim1);  // attaches Tim1() as a timer overflow interrupt
  //-----------------------------------------------------------------------------------
  lcd.clear();
  lcd.print("Position: ");  
  lcd.print(turntable.stepToAngle(turntable.currentPosition()));  
} // end steup


void loop(){ 
  state=1;
  if(irrecv.decode(&resultsIR)){
    irrecv.resume(); // Receive the next value
   
    if (resultsIR.value == LG_OFF){ //Bouton off
      enableMotor = !enableMotor ;
      digitalWrite(EN,enableMotor);
    }
    else if(resultsIR.value == LG_RA){ // Bouton flèche Haut
      turntable.move(turntable.angleToStep(1)); 
      while (turntable.distanceToGo()!=0)turntable.run();  
    }
    else if(resultsIR.value == LG_DA){ // Bouton flèche Haut
      turntable.move(-turntable.angleToStep(1)); 
       while (turntable.distanceToGo()!=0)turntable.run();    
    }
    else if(resultsIR.value == LG_INPUT){ // Bouton Imput
      lastState = state;
      state=2;
      delay(500);
      conf.expTime = acqNum(3,conf.expTime);
      saveEEPROM();   
    }
    else if(resultsIR.value == LG_MENU){ // Bouton Q.Menu
      lastState = state;
      state=3;
      delay(500);
      turntable.setAcceleration(conf.accel=acqNum(3,conf.accel));
      saveEEPROM(); 
    } 
    else if(resultsIR.value == LG_INFO){ // Bouton Info
      lastState = state;
      state=4;
      delay(500);
      turntable.setMaxSpeed(conf.max_Speed=acqNum(4,conf.max_Speed)); 
      saveEEPROM();  
      
    }  
    else if(resultsIR.value == LG_TEXT){ // Bouton Text
      lcd.clear();
      lcd.print("Reset Param");
      conf = {1,1,1,400,750};
      EEPROM.put(ADRESS_CONF,conf.mem); 
      delay(1000);
    } 
    else if(resultsIR.value == LG_HOME){ // Bouton Home
      lcd.clear();
      lcd.print("Interval ?");  
      conf.interv = acqNum(3,conf.interv);
      lcd.clear();
      lcd.print("Temps d expo ?");
      conf.expTime = acqNum(3,conf.expTime);
      turntable.runSnap(conf.interv);
    }
    else if(resultsIR.value == LG_NUM1){ // Bouton 1
      lcd.clear();
      lcd.print("Preset1: Run 10 ");
      delay(500);
      turntable.runSnap(10);
    }
    else if(resultsIR.value == LG_NUM2){ // Bouton 2
      lcd.clear();
      lcd.print("Preset2: Run 15 ");
      delay(1000);
      turntable.runSnap(15);
    }
    else if(resultsIR.value == LG_NUM3){ // Bouton 3
      lcd.clear();
      lcd.print("Preset3: Run 20");
      delay(1000);
      turntable.runSnap(20);
    }
    else if(resultsIR.value == LG_NUM4){ // Bouton 4
      lcd.clear();
      lcd.print("Preset4: Run 30");
      delay(1000);
      turntable.runSnap(30);
    }
    else if(resultsIR.value == LG_NUM5){ // Bouton 5
      lcd.clear(); 
      lcd.print("Preset5: Run 90");
      delay(1000);
      turntable.runSnap(90);
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
      turntable.goTo(acqNum(3,0xfff));    
    } //end else if
  }// end if IRrecv
  digitalWrite (GLED, !enableMotor);  
}// end Main loop

// ---------------------------------- Interrupts ------------------------------------------------

/*void Tim1(){
  digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
  switch (state){
    case 0: // initalisation
      break;
    case 1: // IDLE
      lcd.clear();
      lcd.print("Position: ");  
      lcd.print(turntable.stepToAngle(turntable.currentPosition()));
      lcd.setCursor(11,2);
      lcd.print("St:");
      lcd.print(state);
      break;      
    case 2: // Configuration ExpoTime
      lcd.clear();
      lcd.print("Time: ");
      lcd.print(conf.expTime);
      lcd.print(" second"); 
      break;
    case 3: // Configuration Acceleration
      lcd.clear();
      lcd.print("Accel: ");
      lcd.print(conf.accel);    
      break;
    case 4: // Configuration MaxSpeed
      lcd.clear();
      lcd.print("Max Speed: ");
      lcd.print(conf.max_Speed);
      break;
    case 5: // Go to
      if(irrecv.decode(&resultsIR)){
        irrecv.resume(); // Receive the next value
        if (resultsIR.value == LG_EXIT){ //Bouton EXIT
          Serial.println("EXIT");
          turntable.setCurrentPosition(turntable.distanceToGo());
        }//end if result
      }//end if decode
      lcd.clear();
      lcd.print("Position: ");  
      lcd.print(turntable.stepToAngle(turntable.currentPosition()));
      lcd.setCursor(0,2);
      lcd.print("Go to: ");
      lcd.print(turntable.stepToAngle(turntable.targetPosition()));
      lcd.setCursor(11,2);
      lcd.print("St:");
      lcd.print(state);
    
      break;
    case 6: // RunSnap
      if(irrecv.decode(&resultsIR)){
        irrecv.resume(); // Receive the next value
        if (resultsIR.value == LG_EXIT){ //Bouton EXIT
          Serial.println("EXIT");
          turntable.setCurrentPosition(200*ST_MOD*RP);
      }//end if result
    }//end if decode
      break;
    case 7: // Snap
      if(irrecv.decode(&resultsIR)){
        irrecv.resume(); // Receive the next value
        if (resultsIR.value == LG_EXIT){ //Bouton EXIT
          Serial.println("EXIT");
          turntable.setCurrentPosition(200*ST_MOD*RP);
      }//end if result
    }//end if decode
      break;
    case 8: // InitPosition
      break;
    case 9: // AcquNum 
      lcd.setCursor(11,2);
      lcd.print("St:");
      lcd.print(lastState);
    
      break;
    case 10: // Set Mode
      break;
    default:
      break;        
  }//end switch 
}// end Tim1

*/
//---------------------------------- function acqNum ----------------------------------------------
int acqNum(int numbChar,int ExitValue){ 
  lastState= state;
  state=9;
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
          i=max(i-1,0); //empêche l'indice d'être négatif en cas de spam
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

//--------------------------------------------- function saveEEPROM ------------------------------
void saveEEPROM (){
  conf.mem = false ;
  EEPROM.put(ADRESS_CONF,conf);
  return;
}// end function saveEEPROM
//--- ------------------------------------------function loadEEPROM ------------------------------
void loadEEPROM (){
  if(!EEPROM.get(ADRESS_CONF,conf.mem)){
    conf=EEPROM.get(ADRESS_CONF,conf);
    lcd.clear();
    lcd.print("conf Loaded");
    delay(500);
  }//end if
  return;
} // end function loadEEPROM



