#include <IRremote.h>

IRsend irsend;

void setup() {
}

void loop() {
for (int i = 0; i < 3; i++) {
      irsend.sendLG(551494365, 32);
      delay(40);
    }
                    
  delay(5000);
  
}





