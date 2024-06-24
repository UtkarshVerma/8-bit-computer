#include <Arduino.h>
#include <HardwareSerial.h>
#include <stdbool.h>
#include <stdint.h>

#include "eeprom_programmer.h"
#include "util.h"

constexpr auto INSTRUCTION_MASK = MASK(3, 0);
constexpr auto STEP_MASK        = MASK(6, 4);
constexpr auto BYTE_MASK        = MASK(7, 7);

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

    INSTRUCTION_COUNT,
} instruction;

static const uint16_t microcode[INSTRUCTION_COUNT][8] = {
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

constexpr unsigned int BUFFER_SIZE =
    (BYTE_MASK | STEP_MASK | INSTRUCTION_MASK) + 1;
static uint8_t buffer[BUFFER_SIZE];

static void generate_microcode(void) {
    Serial.print("Generating microcode");
    for (unsigned int instruction = 0; instruction < INSTRUCTION_COUNT;
         ++instruction) {
        bool reached_end = false;
        for (unsigned short step = 0; step < ARRAY_SIZE(microcode[0]);
             ++step) {
            uint16_t micro_instruction = microcode[instruction][step];
            if (reached_end) {
                micro_instruction = 0;
            } else if (micro_instruction == 0) {
                reached_end = true;
            }

            const uint8_t address       = step << 4 | instruction;
            buffer[address | BYTE_MASK] = micro_instruction >> 8;
            buffer[address]             = micro_instruction & 0xff;
        }

        Serial.print(".");
    }
    Serial.println(" done");
}

void setup(void) {
    Serial.begin(115200);
    Serial.println();

    eeprom_programmer_init();

    generate_microcode();
    eeprom_programmer_write(buffer, ARRAY_SIZE(buffer));
    eeprom_programmer_dump(ARRAY_SIZE(buffer));
}

void loop(void) {}
