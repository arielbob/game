#ifndef UTILS_H
#define UTILS_H

uint32 set_bits(uint32 field, uint32 mask, bool32 enabled) {
    // disable the bits
    uint32 result = (field & ~(mask));

    if (enabled) {
        // enable them
        result |= mask;
    }
    return result;
}

#endif
