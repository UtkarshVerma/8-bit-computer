#include <Arduino.h>
#include <HardwareSerial.h>
#include <stdbool.h>
#include <stdint.h>

#include "eeprom_programmer.h"
#include "util.h"

typedef enum {
    A   = BIT(4),
    B   = BIT(3),
    C   = BIT(2),
    D   = BIT(1),
    E   = BIT(0),
    F   = BIT(5),
    G   = BIT(6),
    DOT = BIT(7),
} display_pin;

typedef enum {
    ONES,
    TENS,
    HUNDREDS,
    SIGN,

    DISPLAY_COUNT,
} display;

typedef enum {
    UNSIGNED,
    SIGNED,

    SYMBOL_TYPE_COUNT,
} symbol_type;

typedef enum {
    ZERO,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,

    DECIMAL,
    MINUS,

    SYMBOL_COUNT,
} symbol;

typedef enum {
    NUMBER_COUNT = 256,
} number;

constexpr auto DISPLAY_POS     = 8;
constexpr auto SYMBOL_TYPE_POS = 10;

constexpr uint8_t symbols[SYMBOL_COUNT] = {
    [ZERO]  = A | B | C | D | E | F,
    [ONE]   = B | C,
    [TWO]   = A | B | G | E | D,
    [THREE] = A | B | C | D | G,
    [FOUR]  = B | C | F | G,
    [FIVE]  = A | C | D | F | G,
    [SIX]   = A | C | D | E | F | G,
    [SEVEN] = A | B | C,
    [EIGHT] = A | B | C | D | E | F | G,
    [NINE]  = A | B | C | D | F | G,

    [DECIMAL] = DOT,
    [MINUS]   = G,
};

static uint8_t decode_number(const uint8_t number, const display display,
                             const symbol_type type) {
    uint8_t magnitude = number;
    bool is_negative  = false;
    switch (type) {
        case UNSIGNED:
            break;
        case SIGNED:
            if ((int8_t)number < 0) {
                magnitude   = -(int8_t)number;
                is_negative = true;
            }
            break;
        case SYMBOL_TYPE_COUNT:
            __builtin_unreachable();
    }

    switch (display) {
        case HUNDREDS:
            magnitude /= 10;
            // fallthrough.
        case TENS:
            magnitude /= 10;
            // fallthrough.
        case ONES:
            return symbols[ZERO + (magnitude % 10)];
        case SIGN:
            if (is_negative) {
                return symbols[MINUS];
            }
            break;
        case DISPLAY_COUNT:
            __builtin_unreachable();
    }

    return 0;
}

static void generate_data(uint8_t buffer[NUMBER_COUNT], const display display,
                          const symbol_type type) {
    for (unsigned int num = 0; num < NUMBER_COUNT; ++num) {
        buffer[num] = decode_number((uint8_t)num, display, type);
    }
}

static void program_eeprom(void) {
    Serial.print("Programming EEPROM");
    for (unsigned short place = 0; place < DISPLAY_COUNT; ++place) {
        for (unsigned short type = 0; type < SYMBOL_TYPE_COUNT; ++type) {
            // NOTE: Having a single large buffer exceeds Nano's RAM.
            uint8_t buffer[NUMBER_COUNT];
            generate_data(buffer, (display)place, (symbol_type)type);

            const uint16_t base_address =
                place << DISPLAY_POS | type << SYMBOL_TYPE_POS;
            eeprom_programmer_write(base_address, buffer, ARRAY_SIZE(buffer));
            Serial.print(".");
        }
    }
    Serial.println(" done");
}

static void dump_eeprom(void) {
    Serial.println("Reading EEPROM");
    eeprom_programmer_dump(0,
                           DISPLAY_COUNT * SYMBOL_TYPE_COUNT * NUMBER_COUNT);
}

void setup(void) {
    Serial.begin(115200);
    Serial.println();

    eeprom_programmer_init();

    program_eeprom();
    dump_eeprom();
}

void loop(void) {}
