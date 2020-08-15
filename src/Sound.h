#ifndef SOUND_HEADER
#define SOUND_HEADER
#include "Arduino.h"
#include <stdint.h>

/**
 * The Sound struct. Contains the data to represent a sound.
 */
struct Sound {
    const uint8_t* data = nullptr;
    size_t length = 0;
    size_t location = 0;
    uint16_t remainder = 0;
    uint16_t step = 0;
    uint16_t remainderMask = 0;
    uint16_t remainderShift = 0;
};

#endif
