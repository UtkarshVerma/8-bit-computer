#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define BIT(n)          (1 << (n))
#define MASK(msb, lsb)  ((BIT((msb + 1) - lsb) - 1) << lsb)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // UTIL_H
