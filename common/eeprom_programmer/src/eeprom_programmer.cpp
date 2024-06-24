#include "eeprom_programmer.h"

#include <Arduino.h>
#include <HardwareSerial.h>
#include <stdint.h>
#include <stdio.h>

#include "eeprom.h"
#include "shift_register.h"
#include "util.h"

static const shift_register_config address_shifter = {
    .mosi_pin  = 11,
    .sck_pin   = 13,
    .latch_pin = 10,
};

static const eeprom_config eeprom = {
    .address_shifter = &address_shifter,
    .write_en_pin    = 8,
    .output_en_pin   = 9,
    .data_pins       = {14, 15, 16, 17, 4, 5, 6, 7},
};

void eeprom_programmer_init(void) {
    (void)shift_register_init(&address_shifter);
    (void)eeprom_init(&eeprom);
}

void eeprom_programmer_dump(const uint16_t register_count) {
    Serial.println("Reading EEPROM");
    const unsigned short elements_per_line = 16;
    for (unsigned int base = 0; base < register_count;
         base += elements_per_line) {
        uint8_t data[16];
        for (unsigned short offset = 0; offset < elements_per_line; offset++) {
            data[offset] = eeprom_read(&eeprom, base + offset);
        }

        char buffer[ARRAY_SIZE("000:") +
                    elements_per_line * ARRAY_SIZE(" 00") + 1];
        (void)sprintf(buffer, "%03x: ", base);
        for (unsigned short i = 0; i < elements_per_line; ++i) {
            (void)sprintf(buffer, "%s %02x", buffer, data[i]);

            if (i % 8 == 7) {
                (void)sprintf(buffer, "%s ", buffer);
            }
        }
        Serial.println(buffer);
    }
}

void eeprom_programmer_write(const uint8_t* const buffer,
                             const uint16_t size) {
    Serial.print("Programming EEPROM");
    for (unsigned int page_base = 0; page_base < size;
         page_base += EEPROM_PAGE_SIZE) {
        const unsigned int bound = page_base + EEPROM_PAGE_SIZE > size
                                       ? size - page_base
                                       : EEPROM_PAGE_SIZE;
        for (unsigned int offset = 0; offset < bound; ++offset) {
            const uint16_t address = page_base + offset;
            eeprom_write(&eeprom, address, buffer[address]);
        }
        eeprom_wait(&eeprom);
        Serial.print(".");
    }
    Serial.println(" done");
}
