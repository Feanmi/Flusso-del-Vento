#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

static inline void set_flag_bit(uint8_t* flags, uint8_t pos, uint8_t value) {
    uint8_t mask = 1u << pos;
    *flags = (*flags & ~mask) | (value << pos);
}

typedef struct {
    // Primary measurement data
    uint16_t distance_mm;           // Distance in millimeters
    uint8_t distance_valid;

} telemetry_temp_t;

#endif //COMMON_H
