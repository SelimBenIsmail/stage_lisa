#include <IRremote.h>
int RECV_PIN = 0;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup() {
  CLKPR = 0x80;
  CLKPR = CLKPR|0x07;
  pinMode(3, OUTPUT);
  pinMode(0, INPUT); 
  irrecv.enableIRIn(); // Start the receiver
}


// the loop function runs over and over again forever
void loop() {
  if (irrecv.decode(&results)) {
    irrecv.resume(); // Receive the next value
    while (results.value){
      digitalWrite(3, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(3000);             // wait for a second
      digitalWrite(3, LOW);    // turn the LED off by making the voltage LOW
      delay(1000);             // wait for a second       
      //irrecv.decode(&results);          
    }
  }
}





