#include <Arduino.h>
#include <HardwareSerial.h>
#include <stdbool.h>
#include <stdint.h>

#include "eeprom-programmer.h"
#include "op-code.h"
#include "util.h"

typedef enum {
    FI = BIT(2),   // Flag register in.
    J  = BIT(1),   // Jump.
    CO = BIT(0),   // Program counter out.
    CE = BIT(3),   // Program counter enable.
    OI = BIT(4),   // Output register in.
    BI = BIT(5),   // B register in.
    SU = BIT(6),   // ALU subtract.
    EO = BIT(7),   // ALU out.
    AO = BIT(10),  // A register out.
    AI = BIT(9),   // A register in.
    II = BIT(8),   // Instruction register in.
    IO = BIT(11),  // Instruction register out.
    RO = BIT(12),  // RAM data out.
    RI = BIT(13),  // RAM data in.
    MI = BIT(14),  // Memory address register in.
    HL = BIT(15),  // Halt.
} control_signal;

typedef enum {
    LOWER_BYTE,
    UPPER_BYTE,

    BYTE_INDEX_COUNT,
} byte_index;

typedef enum {
    STEP_COUNT = 8,
} step;

typedef enum {
    CARRY_FLAG,
    ZERO_FLAG,

    FLAG_COUNT = 2,
} flag;

typedef struct {
    uint8_t buffer[BYTE_INDEX_COUNT][STEP_COUNT][OP_CODE_COUNT];
} microcode_template;

static const uint16_t fetch_cycle[] = {MI | CO, RO | II | CE};

typedef struct {
    bool is_conditional;

    // Conditional instructions will only execute on these flags.
    uint8_t flags;

    // Steps in the microcode after the fetch cycle.
    uint16_t steps[STEP_COUNT - ARRAY_SIZE(fetch_cycle)];
} microcode_metadata;

static const microcode_metadata microcode[OP_CODE_COUNT] = {
    [NOP] =
        {
               .is_conditional = false,
               .flags          = 0,
               .steps          = {0, 0, 0, 0, 0, 0},
               },
    [LDA] =
        {
               .is_conditional = false,
               .flags          = 0,
               .steps          = {IO | MI, RO | AI, 0, 0, 0, 0},
               },
    [ADD] =
        {
               .is_conditional = false,
               .flags          = 0,
               .steps          = {IO | MI, RO | BI, EO | AI | FI, 0, 0, 0},
               },
    [SUB] =
        {
               .is_conditional = false,
               .flags          = 0,
               .steps          = {IO | MI, RO | BI, EO | AI | SU | FI, 0, 0, 0},
               },

    [STA] =
        {
               .is_conditional = false,
               .flags          = 0,
               .steps          = {IO | MI, AO | RI, 0, 0, 0, 0},
               },

    [LDI] =
        {
               .is_conditional = false,
               .flags          = 0,
               .steps          = {IO | AI, 0, 0, 0, 0, 0},
               },

    [ADI] =
        {
               .is_conditional = false,
               .flags          = 0,
               .steps          = {IO | BI, EO | AI | FI, 0, 0, 0, 0},
               },
    [SBI] =
        {
               .is_conditional = false,
               .flags          = 0,
               .steps          = {IO | BI, EO | AI | SU | FI, 0, 0, 0, 0},
               },
    [JMP] =
        {
               .is_conditional = false,
               .flags          = 0,
               .steps          = {IO | J, 0, 0, 0, 0, 0},
               },
    [JC] =
        {
               .is_conditional = true,
               .flags          = BIT(CARRY_FLAG),
               .steps          = {IO | J, 0, 0, 0, 0, 0},
               },
    [JZ] =
        {
               .is_conditional = true,
               .flags          = BIT(ZERO_FLAG),
               .steps          = {IO | J, 0, 0, 0, 0, 0},
               },

    [OUT] =
        {
               .is_conditional = false,
               .flags          = 0,
               .steps          = {AO | OI, 0, 0, 0, 0, 0},
               },
    [HLT] =
        {
               .is_conditional = false,
               .flags          = 0,
               .steps          = {static_cast<uint16_t>(HL), 0, 0, 0, 0, 0},
               },
};

static uint8_t get_byte(const uint16_t micro_instruction,
                        const byte_index byte_index) {
    const uint8_t byte_pos = byte_index * 8;

    return (micro_instruction >> byte_pos) & MASK(7, 0);
}

static void fill_template(microcode_template* const buffer) {
    for (unsigned short op_code = 0; op_code < OP_CODE_COUNT; ++op_code) {
        const microcode_metadata metadata = microcode[op_code];

        for (unsigned short step = 0; step < STEP_COUNT; ++step) {
            const uint16_t micro_instruction =
                step < ARRAY_SIZE(fetch_cycle) ? fetch_cycle[step]
                : metadata.is_conditional
                    ? 0
                    : metadata.steps[step - ARRAY_SIZE(fetch_cycle)];

            for (unsigned short bi = 0; bi < BYTE_INDEX_COUNT; ++bi) {
                buffer->buffer[bi][step][op_code] =
                    get_byte(micro_instruction, (byte_index)bi);
            }
        }
    }
}

static void update_template(microcode_template* const buffer,
                            const uint8_t flags) {
    if (flags == 0) {
        return;
    }

    for (unsigned short op_code = 0; op_code < OP_CODE_COUNT; ++op_code) {
        const microcode_metadata metadata = microcode[op_code];
        if (!metadata.is_conditional) {
            continue;
        }

        for (unsigned short step = 0; step < ARRAY_SIZE(metadata.steps);
             ++step) {
            const uint16_t micro_instruction =
                (flags & metadata.flags) != 0 ? metadata.steps[step] : 0;
            for (unsigned short bi = 0; bi < BYTE_INDEX_COUNT; ++bi) {
                buffer->buffer[bi][ARRAY_SIZE(fetch_cycle) + step][op_code] =
                    get_byte(micro_instruction, (byte_index)bi);
            }
        }
    }
}

static void program_eeprom(void) {
    Serial.print("Programming EEPROM");

    microcode_template buffer;
    fill_template(&buffer);

    for (unsigned short flag_mask = 0; flag_mask < POW2(FLAG_COUNT);
         ++flag_mask) {
        update_template(&buffer, flag_mask);
        eeprom_programmer_write(flag_mask * sizeof(microcode_template),
                                (const uint8_t*)buffer.buffer, sizeof(buffer));
        Serial.print(".");
    }
    Serial.println(" done");
}

static void dump_eeprom(void) {
    Serial.println("Reading EEPROM");
    eeprom_programmer_dump(0, POW2(FLAG_COUNT) * sizeof(microcode_template));
}

void setup(void) {
    Serial.begin(115200);
    eeprom_programmer_init();

    program_eeprom();
    dump_eeprom();
}

void loop(void) {}
