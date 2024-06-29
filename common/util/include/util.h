#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdint.h>

#define BIT(n)          (1 << (n))
#define MASK(msb, lsb)  ((BIT((msb + 1) - lsb) - 1) << lsb)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define POW2(n) BIT(n)

#define HEX_FMT_ELEMENT_COUNT 16
#define HEX_FMT_BUFFER_SIZE \
    (ARRAY_SIZE("000:") + HEX_FMT_ELEMENT_COUNT * ARRAY_SIZE(" 00") + 1)

void format_data_as_hex(char* const buffer, const uint8_t* const data,
                        const uint16_t address, const uint8_t size);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // UTIL_H
