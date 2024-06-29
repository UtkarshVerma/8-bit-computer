#ifndef PROGRAM_H
#define PROGRAM_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdint.h>

#define MEMORY_SIZE 16

// C++ doesn't allow designated initializers for arrays, so we keep the program
// in C space.
extern const uint8_t program[MEMORY_SIZE];

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // PROGRAM_H
