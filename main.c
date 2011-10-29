#include <avr/io.h>
#include <avr/interrupt.h>

#define channels 16 	//Must be multiples of 8 (round up and 
			//don't use the most significant bits 
			//if you use a non-multiple)

#define ShiftRegister DDRD
#define ShiftPort PORTD
#define data (1<<PD5)
#define latch (1<<PD6)
#define clock (1<<PD7)

unsigned char pwmValues[16] ={
  0x08,0x30,0xFF,0x30,0x08,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00	//Exponential Approximations
}; 

volatile unsigned char timingFlag = 0;
volatile unsigned char bcmBuffer[channels];
unsigned char BCMtracker = 0;

void calcBCM(void) {
  for (unsigned char i=0; i<8; i++){ //Cycle though each bit of each pwmValues
    //Track groups of 8 bits which let this be adjusted for more or less shift registers
    for (unsigned char channelAdjustment = 0; channelAdjustment<channels/8; channelAdjustment++){
      unsigned char tempValue = 0;
      for (unsigned char j=0; j<8; j++){
        tempValue >>= 1; //Shift right so as not to overwrite the last bit
        if (pwmValues[j+(channelAdjustment*8)] & (1<<i)){
          tempValue |= (1<<7); //Always set MSB because we're shifting right
        }
      }
      bcmBuffer[i+(channelAdjustment*8)] = tempValue;
    }  
  }
}

void shiftCylon(unsigned char direction){
  if (direction){
    for (unsigned char i=0; i<channels-1; i++){
      pwmValues[i] = pwmValues[i+1];
    }
    pwmValues[channels-1] = 0x00;
  }
  else {
    for (unsigned char i=channels; i>0 ; i--){
      pwmValues[i-1] = pwmValues[i-2];
    }
    pwmValues[0] = 0x00;
  }
  calcBCM();
}

int main(void)
{
  //Setup IO
  ShiftRegister |= (data | latch | clock);	//Set outputs
  ShiftPort &= ~(data | latch | clock);	//Set pins low

  //Prewind the Binary Coded Modulation buffer
  calcBCM();

  //Setup Timer
  cli();		//disable all interrupts
  TCCR0A |= (1<<WGM01);	//Use CTC mode
  TCCR0B |= (1<<CS02);	//Start timer with 256 prescaler
  TIMSK0 |= (1<<OCIE0A);	//Enable the compare A interrupt
  OCR0A = 0x01;		//Set to compare on first count
  sei();		//enable all interrupts

  unsigned char timingBuffer = 0;
  unsigned char direction;  
  while(1)
  {
    if (timingFlag) { 	//This flag get set about 122 times per second
      timingFlag = 0;	//Unset flag for next time
      if (++timingBuffer > 4){
        timingBuffer = 0;
        if (pwmValues[0] != 0x00) direction = 0;
        else if (pwmValues[channels-1] != 0x00) direction = 1;
        shiftCylon(direction);
      }
    }
  }
}

ISR(TIMER0_COMPA_vect){
  //Latch data loaded into the shift register on last interrupt
  ShiftPort |= latch;
  ShiftPort &= ~latch;

  //Set interrupt for next BCM delay value
  	//This is confusing--> it actually sets how
	//long to wait with the currently displayed
	//bits before the next interrupt.
  if (BCMtracker == 0) OCR0A = 0x01;
  else OCR0A <<= 1;
  
  //Tell main loop we're about wait for the largest number
  //  clock ticks in the BCM cycle. This is a good time to
  //  do things in the main loop because they'll have the
  //  max amount of time before being interrupted.
  if (OCR0A == (1<<7)) timingFlag = 1;

  //Increment the BCM tracking index
  BCMtracker ++;
  BCMtracker &= 7;	//Flip back to zero when it gets to 8 

  //Strobe in data for next time
  for (signed char channelAdjustment=(channels/8)-1; channelAdjustment>=0; channelAdjustment--){
    for (unsigned char i=0; i<8; i++){
    
      //Set data pin
      if (bcmBuffer[BCMtracker+(channelAdjustment*8)] & (1<<(7-i))) ShiftPort |= data;
      else ShiftPort &= ~data;

      //Toggle clock pin
      ShiftPort |= clock;
      ShiftPort &= ~clock;
    }
  }
  //Do not Latch, that will happen on the next interrupt  
}
