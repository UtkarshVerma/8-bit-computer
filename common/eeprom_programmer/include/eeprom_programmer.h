#ifndef EEPROM_PROGRAMMER_H
#define EEPROM_PROGRAMMER_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdint.h>

void eeprom_programmer_init(void);
void eeprom_programmer_dump(const uint16_t register_count);
void eeprom_programmer_write(const uint8_t* const buffer, const uint16_t size);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // EEPROM_PROGRAMMER_H
