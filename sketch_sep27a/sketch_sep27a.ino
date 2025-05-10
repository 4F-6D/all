#include<avr/io.h>

void setup() 
{
  DDRB |=(1 << DDB5);  //set PB5 as output
}

void loop() {

   PORTB |= (1 << PB5);    // Turning ON LED connected to PB1
   delay(1000);            //Using Arduino IDE inbuilt delay function to generate delay of 1 second
   PORTB &= ~(1 << PB5);   //Turning the LED off
   delay(1000);
}
