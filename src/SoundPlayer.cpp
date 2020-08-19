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

// "Lookup Tables" for doing division by 0-5.
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
    // Set compare match register to the user passed Frequency.
    OCR2A = freq;
    // Turn on CTC mode, which triggers the interrupt on match to OCR2A.
    TCCR2A |= (1 << WGM21);
    // Set CS21 bit for 8 Prescaler (All sampling frequencies land in this range :)).
    TCCR2B |= (1 << CS21);   
    // Enable the interrupt...
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
    
    // We compute the frequency and set the read mode...
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
        stop(); // We can only play 5 notes at once, if we excede that threshold, stop.
        return;
    }
    // These multipliers allow us to avoid performing division in the interrupt.
    divMultiplier = divMultiplierOpt[length];
    divBitshift = divBitshiftOpt[length];
    // Init the sound array and length.
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
    // We use a 32-bit integer to represent our "fixed-decimal" format. The 1st 16bits are 
    // the decimal fraction, and the upper 16 are the actual whole number part. This gives 
    // us plenty of precision while not being super slow like 32bit floats.
    sound->length = ((uint_fast32_t)length) << 16;
    // Compute the array sub-step for this frequency given the sound player sample frequency...
    switch(unit) {
        case HERTZ: 
            sound->step = (soundDuration * sound->length) / sampleFreq;
            break;
        case MILLISECONDS:
            sound->step = ((1000 / soundDuration) * sound->length) / sampleFreq;
            break;
        case MICROSECONDS:
            sound->step = ((1e6 / soundDuration) * sound->length) / sampleFreq;
            break;
        default:
            // Casting required or compiler tries to fit the 1 in a 16-bit integer which causes overflow...
            sound->step = (((uint_fast32_t)1) << 16);
    }
    sound->location = 0;
}

void SoundPlayer::setSoundDuration(Sound *sound, float soundDuration, TimeUnit unit = HERTZ) {
    // We copy the location across as the underlying sound array has not changed...
    // This stops popping sounds from occuring between notes.
    uint_fast32_t loc = sound->location;
    // Length has to be bitshifted back down as it is stored as a 'decimal' with 16 first bits being the decimal.
    setSound(sound, sound->data, (sound->length >> 16), soundDuration, unit);
    sound->location = loc;
}

ISR(TIMER2_COMPA_vect) {
    /*
     * The interrupt timer, plays any notes if they are available....
     * 
     * Note we have to avoid performing any division or modulo in this method
     * as they take over 200 cycles to perform on the ATMEGA (which is stupid.)
     */
    if(soundPtr == nullptr) {
        return;
    }
    
    uint_fast16_t sound = 0;
    for(int i = 0; i < soundPtrSize; i++) {
        Sound *tmp = soundPtr[i];
        //Serial.println(tmp->step);
        size_t loc = tmp->location >> 16;
        sound += (readMode)? (uint_fast8_t)pgm_read_byte(tmp->data + loc): tmp->data[loc];
        tmp->location += tmp->step;
        tmp->location = (tmp->location < tmp->length)? tmp->location: tmp->location - tmp->length;
    }
    
    // Division via (val * (2^8 / length) / 2^8). We can cheat on 0, 1, 2, and 4.
    uint8_t soundOut = (sound * divMultiplier) >> divBitshift; 
    
    PORTD = (PORTD & lowerMask) | (soundOut << soundOutOffset);
    PORTB = (PORTB & upperMask) | (soundOut >> (8 - soundOutOffset));
}
