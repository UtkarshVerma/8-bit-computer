#include "eeprom.h"

#include <Arduino.h>
#include <stdbool.h>
#include <stdint.h>

#include "shift_register.h"
#include "util.h"

static uint8_t last_byte;

static void set_data_bus_mode(const eeprom_config* const config,
                              uint8_t mode) {
    for (unsigned short i = 0; i < ARRAY_SIZE(config->data_pins); ++i) {
        pinMode(config->data_pins[i], mode);
    }
}

int eeprom_init(const eeprom_config* const config) {
    // Ensure that WE outputs HIGH by default.
    pinMode(config->write_en_pin, INPUT_PULLUP);
    pinMode(config->write_en_pin, OUTPUT);

    // Ensure that OE outputs LOW by default.
    digitalWrite(config->output_en_pin, LOW);
    pinMode(config->output_en_pin, OUTPUT);

    // Have the data pins in high-impedance mode by default.
    set_data_bus_mode(config, INPUT);

    return 0;
}

uint8_t eeprom_read(const eeprom_config* const config,
                    const uint16_t address) {
    shift_register_write(config->address_shifter, address);

    uint8_t data = 0;
    for (unsigned short i = 0; i < ARRAY_SIZE(config->data_pins); ++i) {
        const bool bit = digitalRead(config->data_pins[i]);
        data |= bit << i;
    }

    return data;
}

// For page writs, successive writes should be loaded within 150 us.
// (Section 4.3, AT28C64B Datasheet)
void eeprom_write(const eeprom_config* const config, const uint16_t address,
                  const uint8_t data) {
    digitalWrite(config->output_en_pin, HIGH);  // Disable output.

    // Minimum address hold time is 50 ns. (Section 16, AT28C64B Datasheet)
    shift_register_write(config->address_shifter, address);

    // Minimum data setup time is 50 ns before the end of write pulse.
    // (Section 16, AT28C64B Datasheet)
    set_data_bus_mode(config, OUTPUT);
    for (unsigned short i = 0; i < ARRAY_SIZE(config->data_pins); ++i) {
        const bool bit = (data & BIT(i)) != 0;
        digitalWrite(config->data_pins[i], bit);
    }

    // Send write pulse. Must be at least 50 ns wide. No delay is required
    // since a single instruction on Nano takes 62.5 ns (16 MHz).
    // (Section 16, AT28C64B Datasheet)
    digitalWrite(config->write_en_pin, LOW);
    digitalWrite(config->write_en_pin, HIGH);

    // No need to hold the data. (Section 16, AT28C64B Datasheet)
    set_data_bus_mode(config, INPUT);

    digitalWrite(config->output_en_pin, LOW);  // Enable output.

    last_byte = data;
}

void eeprom_wait(const eeprom_config* const config) {
    const unsigned short data_pin_count = ARRAY_SIZE(config->data_pins);

    // Use I/O7 polling to check for pending write.
    const uint8_t msb     = last_byte >> (data_pin_count - 1);
    const uint8_t msb_pin = config->data_pins[data_pin_count - 1];

    // Max write cycle time can be 10 ms. (Section 16, AT28C64B Datasheet)
    while (digitalRead(msb_pin) != msb) {
        delay(1);
    }
}
