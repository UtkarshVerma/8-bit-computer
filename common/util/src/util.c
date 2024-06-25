#include "util.h"

#include <stdint.h>
#include <stdio.h>

void format_data_as_hex(char *const buffer, const uint8_t *const data,
                        const uint16_t address, const uint8_t size) {
    (void)sprintf(buffer, "%03x: ", address & ~(HEX_FMT_ELEMENT_COUNT - 1));

    const unsigned short skipped = address % HEX_FMT_ELEMENT_COUNT;
    for (unsigned short i = 0; i < skipped + size; ++i) {
        if (i < skipped) {
            (void)sprintf(buffer, "%s   ", buffer);
        } else {
            (void)sprintf(buffer, "%s %02x", buffer, data[i]);
        }

        if (i % 8 == 7) {
            (void)sprintf(buffer, "%s ", buffer);
        }
    }
}
