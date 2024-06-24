#ifndef EEPROM_H
#define EEPROM_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdint.h>

#include "shift_register.h"

#define EEPROM_PAGE_SIZE 64

typedef struct {
    const shift_register_config* address_shifter;
    uint8_t write_en_pin;
    uint8_t output_en_pin;
    uint8_t data_pins[8];
} eeprom_config;

int eeprom_init(const eeprom_config* const config);
void eeprom_write(const eeprom_config* const config, const uint16_t address,
                  const uint8_t data);
uint8_t eeprom_read(const eeprom_config* const config, const uint16_t address);
void eeprom_wait(const eeprom_config* const config);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // EEPROM_H
