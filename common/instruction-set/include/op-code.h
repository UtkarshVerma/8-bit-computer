#ifndef OP_CODE_H
#define OP_CODE_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef enum {
    NOP,
    LDA,
    ADD,
    SUB,
    STA,
    LDI,
    ADI,
    SBI,
    JMP,
    JC,
    JZ,
    OUT,
    HLT,

    OP_CODE_COUNT = 16,
} op_code;

#define OP_CODE_POS 4

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // OP_CODE_H
