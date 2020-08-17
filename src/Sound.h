#ifndef SOUND_HEADER
#define SOUND_HEADER
#include "Arduino.h"
#include <stdint.h>

/**
 * The Sound struct. Contains the data to represent a sound.
 */
struct Sound {
    const uint8_t* data = nullptr;
    uint_fast32_t length = 0;
    uint_fast32_t location = 0;
    uint_fast32_t step = 0;
};

#endif
