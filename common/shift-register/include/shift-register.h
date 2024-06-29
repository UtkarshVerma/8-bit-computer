#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdint.h>

// NOTE:
// Hook up MOSI and SCK to SER and SRCLK respectively. MISO can't be used for
// anything.
typedef struct {
    const uint8_t mosi_pin;
    const uint8_t sck_pin;
    const uint8_t latch_pin;
} shift_register_config;

int shift_register_init(const shift_register_config* const config);
void shift_register_write(const shift_register_config* const config,
                          const uint16_t data);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // SHIFT_REGISTER_H
