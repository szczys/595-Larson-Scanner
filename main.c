#include <avr/io.h>
#include <avr/interrupt.h>

#define ShiftRegister DDRD
#define ShiftPort PORTD
#define data (1<<PD5)
#define latch (1<<PD6)
#define clock (1<<PD7)

unsigned char statusBit = 1;
unsigned char slowdown = 0;

int main(void)
{
  //Setup IO
  ShiftRegister |= (data | latch | clock);	//Set outputs
  ShiftPort &= ~(data | latch | clock);	//Set pins low

  //Setup Timer
  cli();		//disable all interrupts
  TCCR0A |= (1<<WGM01);	//Use CTC mode
  TCCR0B |= (1<<CS02) | (1<<CS00);//Start timer with 1024 prescaler
  TIMSK0 |= (1<<OCIE0A);	//Enable the compare A interrupt
  OCR0A = 0xFF;		//Set to compare to TOP
  sei();		//enable all interrupts

  while(1)
  {
    //Loop Forever
  }
}

ISR(TIMER0_COMPA_vect){
  if (++slowdown > 8){
    //Set data pin
    if (statusBit) ShiftPort |= data;
    else ShiftPort &= ~data;


    //Toggle clock pin
    ShiftPort |= clock;
    ShiftPort &= ~clock;
    //latch
    ShiftPort |= latch;
    ShiftPort &= ~latch;

    //prewind for next interrupt
    statusBit ^= 1;
    slowdown = 0;
  }
  
}
