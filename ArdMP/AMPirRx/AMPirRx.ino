#include <IRremote.h>
const uint8_t RECV_PIN = 9;
const uint8_t BLUE_LED = 10;

IRrecv irrecv(RECV_PIN);
decode_results results;

void setup() {
  Serial.begin(9600);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(RECV_PIN, INPUT); 
  digitalWrite(BLUE_LED, LOW);
  irrecv.enableIRIn(); // Start the receiver
  
}

void loop() {
  if (irrecv.decode(&results)) {
    Serial.println(results.value);
    irrecv.resume(); // Receive the next value
    if (results.value == 551494365 ) {
      digitalWrite(BLUE_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(1000);
      digitalWrite(BLUE_LED, LOW);
    }// end if result
  } // end if decode
} // end loop





