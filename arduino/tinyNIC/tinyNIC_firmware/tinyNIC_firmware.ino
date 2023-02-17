// Code optimized for ATTiny85

#include <avr/io.h>
#include <avr/interrupt.h>
#include <EEPROM.h>
#include "fastport.h"
  
// for *speed*, cuz calling and returning a function is *slow*, and we want direct var manipulation without pointers!!
inline void writeNetworkAddress() __attribute__((always_inline));  
inline void readNetworkAddress() __attribute__((always_inline));  
inline void onControllerClock() __attribute__((always_inline));  


#define CTR_CLK  PB3    // ATtiny85 pin 2
#define CTR_DATA PB4    // ATtiny85 pin 3
#define NET_CLK  1
#define NET_TX   1
#define NET_RX   1

uint8_t network_address;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

// Controller clock pulse handler for communication with device controller
uint8_t opcode = 0;     // the opcode to act on
uint8_t op_state = 0;   // the progress state of the current executing opcode/operation
uint8_t bit_buff = 0;   // byte that input bits are read to before being flushed elsewhere
uint8_t bits_read = 0;  // variable to store bits read in for current operation state
uint8_t ctr_data = 0;   // the current value of the controller data pin

void onControllerClock() {

  ctr_data = readPin(CTR_DATA);  

  // Read in the opcode if there isn't one set
  if(!op_state) {

    opcode = opcode << 1;    
    opcode = opcode | ctr_data;

    // Check if the opcode has been fully read in (4th bit set)
    if(opcode & 0x08) {
      opcode = opcode & 0x07;   // strip 4th bit to allow for true opcode value in memory
      op_state = 1;
    } else {
      return;
    }
  }

  // Operate depending on the current opcode
  switch(opcode) {

    case 0:  // write new network address to eeprom and running memory

      writeNetworkAddress();
      break;  
    
    case 1:  // 

      readNetworkAddress();
      break;    

  }
}


// write the network address new_addr to EEPROM and SRAM
void writeNetworkAddress() {

  switch(op_state) {

    case 1:   // if we finished reading in the opcode on this cycle, wait for the next cycle to read in data
      op_state = 2;
      return;

    case 2:   // keep reading new network address into bit_buff until the whole byte is read in

      // read in a byte bit by bit
      bit_buff = bit_buff << 1;
      bit_buff = bit_buff | ctr_data;
      bits_read += 1;

      // if the entire byte has been read, write it to memory EEPROM and advance the operation state
      if (bits_read == 8) {

        //EEPROM.update(0, bit_buff);   // need a register manipulation polling method, this uses interrupts
        network_address = bit_buff;
        op_state = 3;

        // Get ready to return a true bit!
        setPinModeOutput(CTR_DATA);
        setPinHigh(CTR_DATA);
      }

      break;

    case 3:   // reset everything

      op_state = 0;
      opcode = 0;
      bit_buff = 0;
      bits_read = 0;
      setPinLow(CTR_DATA);
      setPinModeInput(CTR_DATA);

  }
}


// Read out the currently set network address to the device controller
void readNetworkAddress() {
  
  uint8_t output = 0;   // would it be faster if this were a global??

  switch (op_state) {

    case 1:     // if we finished reading in the opcode, prepare to send data

      setPinModeOutput(CTR_DATA);
      bit_buff = network_address;
      bits_read = 8;
      op_state = 2;
    

    case 2:     // write data to output on every clock pulse

      output = bit_buff & 0x01;
      if (output) {
        setPinHigh(CTR_DATA);
      } else {
        setPinLow(CTR_DATA);
      }

      bit_buff = bit_buff >> 1;
      bits_read -= 1;

      if (bits_read == 0) {
        op_state = 3;
      }

      break;


    case 3:     // return a high bit to let the controller know we received the message
      setPinHigh(CTR_DATA);
      op_state = 4;
      break;


    case 4:   // reset everything

      op_state = 0;
      opcode = 0;
      bit_buff = 0;
      bits_read = 0;
      setPinLow(CTR_DATA);
      setPinModeInput(CTR_DATA);
      break;

  }
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //


// Network clock pulse handler for communication with network switch
void onNetworkClock() {
  return;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

// Actual Interrupt Service Routine
volatile uint8_t interrupt_flag = 0;
volatile uint8_t portb_history = 0;

ISR (PCINT0_vect) {

  sei();    // enable nested interrupts in case multiple clock toggles have occured

  // Check which pin triggered the pin change interrupt by comparing its last state
  uint8_t changed_bits = PINB ^ portb_history;
  portb_history = PINB;

  // if the controller clock changed state
  if (changed_bits & (1 << CTR_CLK)) {
    interrupt_flag |= 1;    // set first bit of flag if unset  
  }

  // if the network clock changed state
  if (changed_bits & (1 << NET_CLK)) {
    interrupt_flag |= 2;    // set second bit of flag if unset
  }

}



void setup() {

  // network_address = EEPROM.read(0);   // unsaved. function for writing to EEPROM still needs work

  // Set initial pin modes
  setPinModeInput(CTR_CLK);
  setPinModeInput(CTR_DATA);


  // Enable and set relevant interrupts
  GIMSK |= (1 << PCIF);     // enable pin change interrupt flag (PCIF) in general interrupt mask register (GIMSK)
  PCMSK |= (1 << PCINT3);   // enable PCINT3 (pin 2) / CTR_CLK
  //PCMSK |= (1 << PCINT2);   // enable PCINT2 (pin 7) / NET_CLK

  sei();    // enable the ability for interrupts to trigger

}


void loop() {

  // Check if the flag set by a controller clock pin state change is set
  if (interrupt_flag & 1) {
    onControllerClock();
    interrupt_flag &= 0xFE;  // reset the flag for the controller clock
  }

  if (interrupt_flag & 2) {
    onNetworkClock();
    interrupt_flag &= 0xFD;
  }
  
}




