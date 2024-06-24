#include "shift_register.h"

#include <Arduino.h>
#include <SPI.h>
#include <stdbool.h>
#include <stdint.h>

int shift_register_init(const shift_register_config* const config) {
    pinMode(config->latch_pin, OUTPUT);
    pinMode(config->mosi_pin, OUTPUT);
    pinMode(config->sck_pin, OUTPUT);

    SPI.begin();

    // Minimum supported clock pulse width is 20 ns. The SPI clock can't go
    // higher than Fosc / 2 which yields a 125 ns clock on Nano @ 16 MHz.
    // Hence, this is safe.
    // Setup and hold times are also under 25 ns for each clock signal which
    // are well within the SPI clock bounds.
    // (Section 6.6, SN74HC595 Datasheet)
    // (Section 18.5.1, ATmega328P Datasheet)
    SPI.beginTransaction(SPISettings(F_CPU, MSBFIRST, SPI_MODE0));

    return 0;
}

void shift_register_write(const shift_register_config* const config,
                          const uint16_t data) {
    (void)SPI.transfer(data >> 8);
    (void)SPI.transfer(data & 0xff);

    // Latch the shifted data into the output register.
    // The shift register supports a minimum of 20 ns pulse width. Even a
    // single clock cycle on the Nano exceeds that (62.5 ns @ 16 MHz). So, no
    // extra delay is required.
    // (Section 6.6, SN74HC595 Datasheet)
    digitalWrite(config->latch_pin, HIGH);
    digitalWrite(config->latch_pin, LOW);
}
