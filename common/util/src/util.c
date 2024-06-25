#include "util.h"

#include <stdint.h>
#include <stdio.h>

void format_data_as_hex(char buffer[HEX_FMT_BUFFER_SIZE],
                        const uint8_t data[HEX_FMT_ELEMENT_COUNT],
                        const uint16_t base_address) {
    (void)sprintf(buffer, "%03x: ", base_address);
    for (unsigned short i = 0; i < HEX_FMT_ELEMENT_COUNT; ++i) {
        (void)sprintf(buffer, "%s %02x", buffer, data[i]);

        if (i % 8 == 7) {
            (void)sprintf(buffer, "%s ", buffer);
        }
    }
}
