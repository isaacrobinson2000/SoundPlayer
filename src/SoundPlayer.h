#ifndef SOUND_PLAYER_HEADER
#define SOUND_PLAYER_HEADER
#include "Arduino.h"
#include "Sound.h"

/**
 * The supported time units by the SoundPlayer class. 
 */
enum TimeUnit: uint8_t {
    HERTZ,
    MILLISECONDS,
    MICROSECONDS,
    NONE // Special Value. Sets the array step to 1, meaning audio is assumed to have been recorded at the sampling frequency.
};

/**
 * The ReadMode of the SoundPlayer, MEMORY means the sounds samples are 
 * stored in RAM, and FLASH means sounds are stored in program memory 
 * using the PROGMEM attribute.
 */
enum ReadMode: bool {
    MEMORY = false,
    FLASH = true
};

/*
 * The sample frequency of the SoundPlayer. Currently supports:
 *  - 40kHz: Can only play 2 sounds max...
 *  - 20kHz
 *  - 16kHz
 *  - 24kHz
 *  - 26kHz
 */
enum SamplingFrequency: uint8_t {
    Hz40k = 49,
    Hz20k = 99,
    Hz16k = 124,
    Hz24k = 82,
    Hz26k = 75
};

/**
 * Capable of playing 8-bit sounds via the Ardiuno Uno pins.
 */
class SoundPlayer {
    public:
        /**
         * Construct a new sound player.
         * 
         * @param offset: The starting pin of the 8-pins to output to. Can be between 2 and 6.
         * @param mode: The ReadMode of this sound player, either FLASH or MEMORY. Defaults to MEMORY.
         * @param freq: The SamplingFrequency of this sound player. Defaults to the 40kHz value (Hz40k).
         */
        SoundPlayer(uint8_t offset, ReadMode mode = MEMORY, SamplingFrequency freq = Hz40k);
        
        /**
         * Stop the SoundPlayer, clearing all currently playing sounds.
         */
        void stop();
        
        /**
         * Change the sound being played to a new sound collection...
         * 
         * @param soundArray: The list of sounds to play.
         * @param length: The length of the soundArray. Note the sound player has a max limit of 5 notes.
         */
        void play(Sound* soundArray[], size_t length);
        
        /**
         * Construct a new sound object using this SoundPlayer.
         * 
         * @param data: An array of 8-bit unsigned integers, representing the sampled sound.
         * @param length: The length of the 8-bit array.
         * @param soundDuration: A float being the duration of the above sound sample, written in one of the supported unit types.
         * @param unit: A value from the TimeUnit enum, being the unit of measurement to use. Defualts to HERTZ.
         * 
         * @return: A new Sound pointer. This sound must be explicitly 
         *          deleted as it is alocated on the heap.
         */
        Sound* newSound(const uint8_t data[], size_t length, float soundDuration, TimeUnit unit = HERTZ);
        
        /**
         * Set the values of a Sound object using this SoundPlayer.
         * 
         * @param sound: The Sound object to change the values of.
         * @param data: An array of 8-bit unsigned integers, representing the sampled sound.
         * @param length: The length of the 8-bit array.
         * @param soundDuration: A float being the duration of the above sound sample, written in one of the supported unit types.
         * @param unit: A value from the TimeUnit enum, being the unit of measurement to use. Defualts to HERTZ.
         */
        void setSound(Sound *sound, const uint8_t data[], size_t length, float soundDuration, TimeUnit unit = HERTZ);
        
        /**
         * Set the frequency of the passed Sound object.
         * 
         * @param sound: The Sound object to change the values of.
         * @param soundDuration: A float being the duration of the above sound sample, written in one of the supported unit types.
         * @param unit: A value from the TimeUnit enum, being the unit of measurement to use. Defualts to HERTZ.
         */
        void setSoundDuration(Sound *sound, float soundDuration, TimeUnit unit = HERTZ);
    
    private:
        float sampleFreq;
};

#endif
