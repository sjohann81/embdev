#include <stdint.h>

uint32_t __mulsi3(uint32_t a, uint32_t b);
uint64_t __muldsi3(uint32_t a, uint32_t b);
uint64_t __muldi3(uint64_t a, uint64_t b);
uint32_t __udivmodsi4(uint32_t num, uint32_t den, int8_t mod);
int32_t __divmodsi4(int32_t num, int32_t den, int8_t mod);
uint32_t __udivsi3(uint32_t num, uint32_t den);
uint32_t __umodsi3(uint32_t num, uint32_t den);
int32_t __divsi3(int32_t num, int32_t den);
int32_t __modsi3(int32_t num, int32_t den);
uint64_t __ashldi3(uint64_t v, int32_t cnt);
uint64_t __ashrdi3(uint64_t v, int32_t cnt);
uint64_t __lshrdi3(uint64_t v, int32_t cnt);
uint64_t __udivmoddi4(uint64_t num, uint64_t den, uint64_t *rem);
int64_t __divmoddi4(int64_t num, int64_t den, int64_t *rem);
uint64_t __umoddi3(uint64_t num, uint64_t den);
uint64_t __udivdi3(uint64_t num, uint64_t den);
int64_t __moddi3(int64_t num, int64_t den);
int64_t __divdi3(int64_t num, int64_t den);
