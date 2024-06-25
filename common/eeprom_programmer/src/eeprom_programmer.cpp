#include "eeprom_programmer.h"

#include <Arduino.h>
#include <HardwareSerial.h>
#include <stdint.h>

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

void eeprom_programmer_read(uint8_t* const buffer, const uint16_t base_address,
                            const uint16_t size) {
    for (uint32_t i = 0; i < size; ++i) {
        buffer[i] = eeprom_read(&eeprom, base_address + i);
    }
}

void eeprom_programmer_write(const uint16_t address,
                             const uint8_t* const buffer,
                             const uint16_t size) {
    for (uint16_t offset = 0; offset < size; ++offset) {
        const uint16_t addr = address + offset;
        eeprom_write(&eeprom, addr, buffer[offset]);

        // Wait if last page write or last write.
        const uint16_t page_offset_mask = EEPROM_PAGE_SIZE - 1;
        if ((addr & page_offset_mask) == page_offset_mask ||
            offset == size - 1) {
            eeprom_wait(&eeprom);
        }
    }
}

void eeprom_programmer_dump(const uint16_t size) {
    for (uint16_t base = 0; base < size; base += HEX_FMT_ELEMENT_COUNT) {
        uint8_t data[HEX_FMT_ELEMENT_COUNT];
        eeprom_programmer_read(data, base, ARRAY_SIZE(data));

        char buffer[HEX_FMT_BUFFER_SIZE];
        format_data_as_hex(buffer, data, base);

        Serial.println(buffer);
    }
}
