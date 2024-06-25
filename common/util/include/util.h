#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdint.h>

#define BIT(n)          (1 << (n))
#define MASK(msb, lsb)  ((BIT((msb + 1) - lsb) - 1) << lsb)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define HEX_FMT_ELEMENT_COUNT 16
#define HEX_FMT_BUFFER_SIZE \
    (ARRAY_SIZE("000:") + HEX_FMT_ELEMENT_COUNT * ARRAY_SIZE(" 00") + 1)

void format_data_as_hex(char buffer[HEX_FMT_BUFFER_SIZE],
                        const uint8_t data[HEX_FMT_ELEMENT_COUNT],
                        const uint16_t base_address);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // UTIL_H
