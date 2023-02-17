// Functions to allow for faster port manipulation compared to the standard arduino digitalwrite/reads
// This code is currently written specifically for the ATtiny85 MCU

#include <avr/io.h>

void setPinModeOutput(uint8_t port) {
  DDRB |= (1 << port);	
}

void setPinModeInput(uint8_t port) {
  DDRB &= ~(1 << port);
}

void setPinHigh(uint8_t port) {
  PORTB |= (1 << port);	
}

void setPinLow(uint8_t port) {
  PORTB &= ~(1 << port);	
}

bool readPin(uint8_t port) {
  return PINB & (1 << port);
}
