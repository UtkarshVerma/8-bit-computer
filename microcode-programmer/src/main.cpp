#include <Arduino.h>
#include <HardwareSerial.h>
#include <stdbool.h>
#include <stdint.h>

#include "eeprom_programmer.h"
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
    NOP,
    LDA,
    ADD,
    SUB,
    STA,
    LDI,
    JMP,
    OUT,
    HLT,

    INSTRUCTION_COUNT = 16,
} instruction;

typedef enum {
    LOWER,
    UPPER,

    BYTE_INDEX_COUNT,
} byte_index;

typedef enum {
    STEP_COUNT = 8,
} step;

static const uint16_t microcode[INSTRUCTION_COUNT][STEP_COUNT] = {
    [NOP] = {MI | CO, RO | II | CE, 0},
    [LDA] = {MI | CO, RO | II | CE, IO | MI, RO | AI, 0},
    [ADD] = {MI | CO, RO | II | CE, IO | MI, RO | BI, EO | AI, 0},
    [SUB] = {MI | CO, RO | II | CE, IO | MI, RO | BI, EO | AI | SU, 0},
    [STA] = {MI | CO, RO | II | CE, IO | MI, AO | RI, 0},
    [LDI] = {MI | CO, RO | II | CE, IO | AI, 0},
    [JMP] = {MI | CO, RO | II | CE, IO | J, 0},
    [OUT] = {MI | CO, RO | II | CE, AO | OI, 0},
    [HLT] = {MI | CO, RO | II | CE, static_cast<uint16_t>(HL), 0},
};

static uint8_t buffer[BYTE_INDEX_COUNT * STEP_COUNT * INSTRUCTION_COUNT];

// NOTE: This must be packed for the static buffer allocation to work.
constexpr auto INSTRUCTION_POS = 0;
constexpr auto STEP_POS        = 4;
constexpr auto BYTE_POS        = 7;

static void generate_microcode(void) {
    for (unsigned short instruction = 0; instruction < INSTRUCTION_COUNT;
         ++instruction) {
        bool reached_last_step = false;
        for (unsigned short step = 0; step < STEP_COUNT; ++step) {
            const uint16_t micro_instruction =
                reached_last_step ? 0 : microcode[instruction][step];
            if (micro_instruction == 0) {
                reached_last_step = true;
            }

            for (unsigned short bi = 0; bi < BYTE_INDEX_COUNT; ++bi) {
                const uint8_t byte_pos = bi * 8;
                uint8_t data = (micro_instruction >> byte_pos) & MASK(7, 0);

                const uint16_t address = bi << BYTE_POS | step << STEP_POS |
                                         instruction << INSTRUCTION_POS;
                buffer[address] = data;
            }
        }
    }
}

static void program_eeprom(void) {
    Serial.print("Programming EEPROM");
    const uint8_t chunk_size = INSTRUCTION_COUNT;
    for (uint16_t i = 0; i < ARRAY_SIZE(buffer); i += chunk_size) {
        eeprom_programmer_write(i, &buffer[i], chunk_size);
        Serial.print(".");
    }
    Serial.println(" done");
}

static void dump_eeprom(void) {
    Serial.println("Reading EEPROM");
    eeprom_programmer_dump(ARRAY_SIZE(buffer));
}

void setup(void) {
    Serial.begin(115200);
    eeprom_programmer_init();

    generate_microcode();
    program_eeprom();
    dump_eeprom();
}

void loop(void) {}
