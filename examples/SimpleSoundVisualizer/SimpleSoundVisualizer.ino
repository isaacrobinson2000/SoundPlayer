/**
 * A simple SoundPlayer example.
 * 
 * Plays an 8Hz, 12Hz, and 14Hz signal at the same time, and 
 * reads in the analog signal, printing it on the serial monitor. 
 * 
 * You will need an R-R2 resistor ladder(or any other 8-pin DAC) 
 * to actually see the output of this sketch. To drive a speaker
 * you will also need a driver phase after the R-R2 resistor 
 * ladder or DAC, as often more voltage and current is needed. 
 * (Typically a rail-to-rail op amp hooked up as a voltage 
 *  follower followed by a push-pull amplifier).
 * 
 * Goto "Tools -> Serial Plotter" to see the sine waves generated.
 */
#include <Sound.h>
#include <SoundPlayer.h>
// The analog read pin.
const int analogPin = A3;

// A sine wave made up of 300 samples. Has an amplitude 
// that is 2/3 the maximum possible value (255).
const PROGMEM uint8_t sineWave[] = {
    25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 32, 32, 33, 33,
    34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39, 40, 40, 40, 41, 41,
    42, 42, 43, 43, 43, 44, 44, 44, 45, 45, 45, 46, 46, 46, 47, 47, 47,
    47, 48, 48, 48, 48, 49, 49, 49, 49, 49, 49, 50, 50, 50, 50, 50, 50,
    50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
    50, 50, 50, 50, 49, 49, 49, 49, 49, 48, 48, 48, 48, 48, 47, 47, 47,
    46, 46, 46, 46, 45, 45, 45, 44, 44, 43, 43, 43, 42, 42, 42, 41, 41,
    40, 40, 39, 39, 38, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33,
    32, 32, 31, 31, 30, 30, 29, 28, 28, 27, 27, 26, 26, 25, 25, 24, 24,
    23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15,
    14, 14, 13, 13, 12, 12, 12, 11, 11, 10, 10,  9,  9,  8,  8,  8,  7,
    7,  7,  6,  6,  5,  5,  5,  4,  4,  4,  4,  3,  3,  3,  2,  2,  2,
    2,  2,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  3,  3,  4,  4,  4,
    5,  5,  5,  6,  6,  6,  7,  7,  7,  8,  8,  9,  9, 10, 10, 10, 11,
    11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19,
    20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25
};
const size_t sineWaveLen = sizeof(sineWave) / sizeof(uint8_t);

// SoundPlayer and Sound array declarations...
SoundPlayer *player;
Sound* arr[3];
const int arrLen = sizeof(arr) / sizeof(Sound*);

void setup() {
  // Start the serial monitor...
  Serial.begin(9600);
  // Make a new SoundPlayer using digital ports 2(lsb)-10(msb), outputing sounds with a 
  // sampling frequeucy of 26kHz.
  player = new SoundPlayer(2, FLASH, Hz26k);
  // Create the sounds using the sound player...
  arr[0] = player->newSound(sineWave, sineWaveLen, 8, HERTZ);
  // A 12Hz duration, passed in microseconds instead (milliseconds is also supported).
  arr[1] = player->newSound(sineWave, sineWaveLen, 83333.333, MICROSECONDS);
  // This is also valid(14Hz), the time unit parameter always defaults to HERTZ.
  arr[2] = player->newSound(sineWave, sineWaveLen, 14);
  // Play the sounds....
  player->play(arr, arrLen);
}

void loop() {
  // Read in from the analog pin and send the result to the serial monitor.
  int result = analogRead(analogPin);
  Serial.println(result);
}
