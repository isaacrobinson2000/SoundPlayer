#include "Arduino.h"
#include <stdint.h>
#include "Sound.h"
#include "SoundPlayer.h"

// Static variables. Used to pass sounds to the interrupt timer method.
static volatile Sound **soundPtr = nullptr;
static volatile size_t soundPtrSize = 0;
static volatile uint16_t divMultiplier = 0;
static volatile uint16_t divBitshift = 0;
static volatile uint8_t soundOutOffset = 2;
static volatile uint8_t upperMask = 0;
static volatile uint8_t lowerMask = 0;
static volatile ReadMode readMode = MEMORY;

static const float CLOCK_FREQUENCY = 16000000;

static const uint16_t divMultiplierOpt[] = {0, 1, 1, 85, 1, 51};
static const uint16_t divBitshiftOpt[] = {0, 0, 1, 8, 2, 8};
static const size_t maxSoundCount = (sizeof(divMultiplierOpt) / sizeof(uint16_t)) - 1;

// static const uint16_t STEP_LIMIT = (1 << 15);


SoundPlayer::SoundPlayer(uint8_t offset, ReadMode mode = MEMORY, SamplingFrequency freq = Hz40k) {
    cli(); // Stop all interrupts...
    
    // Clear all timer 2 values...
    TCCR2A = 0;
    TCCR2B = 0;
    TCNT2  = 0;
    // set compare match register for 24khz.
    OCR2A = freq; // = (16*10^6) / (16000*8) - 1 = 124 (must be <256)
    // turn on CTC mode
    TCCR2A |= (1 << WGM21);
    // Set CS21 bit for 8 prescaler
    TCCR2B |= (1 << CS21);   
    // enable timer compare interrupt
    TIMSK2 |= (1 << OCIE2A);

    // Get the offset...
    offset = (offset < 2)? 2: ((offset > 6)? 6: offset);
    soundOutOffset = offset;
    // Compute the lower bitwise mask and the upper bitwise mask to match the bit offset above...
    lowerMask = (1 << offset) - 1;  // Same as pow(2, offset) - 1.
    upperMask = ~lowerMask;
    
    soundPtr = nullptr;
    soundPtrSize = 0;
    
    DDRD = DDRD | upperMask;  // Init lower selected ports (offset-7) as outputs.
    DDRB = DDRB | lowerMask; // Init ports (8 - (offset + 8)) as outputs.
    
    sampleFreq = CLOCK_FREQUENCY / (8 * ((float)freq + 1));
    readMode = mode;

    sei(); // Allow all interrupts run again.
}

void SoundPlayer::stop() {
    cli();
    
    soundPtr = nullptr;
    soundPtrSize = 0;
    
    sei();
}

void SoundPlayer::play(Sound* soundArray[], unsigned int length) {
    cli();
    
    if(length > maxSoundCount) {
        stop();
        return;
    }
    divMultiplier = divMultiplierOpt[length];
    divBitshift = divBitshiftOpt[length];
    soundPtr = soundArray;
    soundPtrSize = length;
    
    sei();
}

Sound* SoundPlayer::newSound(const uint8_t data[], size_t length, float soundDuration, TimeUnit unit = HERTZ) {
    Sound* sound = new Sound();
    setSound(sound, data, length, soundDuration, unit);
    return sound;
}

void SoundPlayer::setSound(Sound *sound, const uint8_t data[], size_t length, float soundDuration, TimeUnit unit = HERTZ) {
    sound->data = data;
    sound->length = length;
    // We use 1024 as our substep fraction value as this allows for really fast division routines (since 1024 = 2^10).
    switch(unit) {
        case HERTZ: 
            sound->step = (soundDuration * length * 512) / sampleFreq;
            break;
        case MILLISECONDS:
            sound->step = ((1000 / soundDuration) * length * 512) / sampleFreq;
            break;
        case MICROSECONDS:
            sound->step = ((1e6 / soundDuration) * length * 512) / sampleFreq;
            break;
        default:
            sound->step = 512;
    }
    sound->location = 0;
    sound->remainder = 0;
}

void SoundPlayer::setSoundDuration(Sound *sound, float soundDuration, TimeUnit unit = HERTZ) {
    setSound(sound, sound->data, sound->length, soundDuration, unit);
}

// static volatile int counter = 0;

ISR(TIMER2_COMPA_vect) {
    /*
     * The interrupt timer, plays any notes if they are available....
     * 
     * Note we have to avoid performing any division or modulo in the method
     * as they take over 200 cycles to perform on the ATMEGA (which is stupid.)
     */
    
    if(soundPtr == nullptr) {
        return;
    }
    
    uint16_t sound = 0;
    for(int i = 0; i < soundPtrSize; i++) {
        Sound* tmp = soundPtr[i];
        sound += (readMode)? (uint8_t)pgm_read_byte(tmp->data + tmp->location): tmp->data[tmp->location];
        tmp->remainder += tmp->step;
        tmp->location += tmp->remainder >> 9; // Same as (remainder / 512)
        tmp->location = (tmp->location < tmp->length)? tmp->location: tmp->location - tmp->length;
        tmp->remainder &= 0b111111111; // Same as (remainder % 512)
    }
    uint8_t soundOut = (sound * divMultiplier) >> divBitshift;
    
    PORTD = (PORTD & lowerMask) | (soundOut << soundOutOffset);
    PORTB = (PORTB & upperMask) | (soundOut >> (8 - soundOutOffset));
}
