#include <Arduino.h>
#include <pins_arduino.h>
#include <stdint.h>

#include "program.h"
#include "util.h"

constexpr uint8_t BUS_PINS[8] = {9, 10, 11, 12, A0, A1, A2, A3};
constexpr uint8_t HALT        = 2;
constexpr uint8_t MEMORY_IN   = 3;
constexpr uint8_t EEPROM_CE   = 4;
constexpr uint8_t RESET       = A4;
constexpr uint8_t CLOCK       = A5;

// Minicore mapps the crystal pins to 20, 21 when using ATmega's internal
// clock.
constexpr uint8_t RAM_IN  = 20;
constexpr uint8_t RAM_OUT = 21;

constexpr uint8_t SIGNED = 13;

constexpr uint8_t control_signals[] = {
    MEMORY_IN, RAM_IN, RAM_OUT, RESET, HALT,
};
constexpr uint8_t config_pins[] = {SIGNED, EEPROM_CE, CLOCK};

static void pulse_pin(const uint8_t pin) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(1);
    digitalWrite(pin, LOW);
}

static void write_to_bus(const uint8_t byte) {
    for (uint8_t i = 0; i < ARRAY_SIZE(BUS_PINS); ++i) {
        digitalWrite(BUS_PINS[i], (byte & BIT(i)) != 0);
    }
}

static void write_to_ram(const uint8_t address, const uint8_t byte) {
    write_to_bus(address);
    digitalWrite(MEMORY_IN, HIGH);
    pulse_pin(CLOCK);
    digitalWrite(MEMORY_IN, LOW);

    write_to_bus(byte);
    digitalWrite(RAM_IN, HIGH);
    pulse_pin(CLOCK);
    digitalWrite(RAM_IN, LOW);
}

static void write_program(void) {
    digitalWrite(HALT, HIGH);       // Halt system clock.
    digitalWrite(EEPROM_CE, HIGH);  // Disable EEPROM.

    for (uint8_t i = 0; i < ARRAY_SIZE(program); ++i) {
        write_to_ram(i, program[i]);
    }

    digitalWrite(EEPROM_CE, LOW);  // Enable EEPROM.
}

void setup(void) {
    // Assert the pins.
    for (uint8_t i = 0; i < ARRAY_SIZE(BUS_PINS); ++i) {
        pinMode(BUS_PINS[i], OUTPUT);
    }
    for (uint8_t i = 0; i < ARRAY_SIZE(control_signals); ++i) {
        pinMode(control_signals[i], OUTPUT);
    }
    for (uint8_t i = 0; i < ARRAY_SIZE(config_pins); ++i) {
        pinMode(config_pins[i], OUTPUT);
    }

    digitalWrite(CLOCK, LOW);  // Assert our clock.
    write_program();
    digitalWrite(SIGNED, LOW);  // Disable signed mode.
    pulse_pin(RESET);           // Reset the computer.

    // Deassert the pins.
    for (uint8_t i = 0; i < ARRAY_SIZE(BUS_PINS); ++i) {
        pinMode(BUS_PINS[i], INPUT);
    }
    for (uint8_t i = 0; i < ARRAY_SIZE(control_signals); ++i) {
        pinMode(control_signals[i], INPUT);
    }
}

void loop(void) {}
