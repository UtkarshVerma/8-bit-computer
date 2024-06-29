#include "program.h"

#include <stdint.h>

#include "op-code.h"

#define INST(op_code, arg) ((op_code) << OP_CODE_POS | (arg))

const uint8_t program[MEMORY_SIZE] = {
    // clang-format off
    [0] = INST(LDA, 15),
    [1] = INST(ADD, 14),
    [2] = INST(JC, 5),
    [3] = INST(OUT, 0),
    [4] = INST(JMP, 1),
    [5] = INST(SUB, 14),
    [6] = INST(OUT, 0),
    [7] = INST(JZ, 1),
    [8] = INST(JMP, 5),
    // clang-format on

    [14] = 1,
    [15] = 1,
};
