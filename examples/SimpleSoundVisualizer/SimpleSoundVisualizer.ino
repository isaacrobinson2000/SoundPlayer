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
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 4, 4, 
  5, 5, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14, 15, 16, 
  17, 18, 19, 20, 21, 23, 24, 25, 26, 28, 29, 30, 
  32, 33, 35, 36, 37, 39, 40, 42, 44, 45, 47, 48, 
  50, 52, 53, 55, 57, 58, 60, 62, 63, 65, 67, 69, 
  70, 72, 74, 76, 77, 79, 81, 83, 84, 86, 88, 90, 
  92, 93, 95, 97, 99, 100, 102, 104, 106, 107, 109, 
  111, 112, 114, 116, 117, 119, 121, 122, 124, 125, 
  127, 129, 130, 132, 133, 134, 136, 137, 139, 140, 
  141, 143, 144, 145, 146, 148, 149, 150, 151, 152, 
  153, 154, 155, 156, 157, 158, 159, 160, 161, 161, 
  162, 163, 164, 164, 165, 165, 166, 166, 167, 167, 
  168, 168, 168, 169, 169, 169, 169, 169, 169, 169, 
  170, 169, 169, 169, 169, 169, 169, 169, 168, 168, 
  168, 167, 167, 166, 166, 165, 165, 164, 164, 163, 
  162, 161, 161, 160, 159, 158, 157, 156, 155, 154, 
  153, 152, 151, 150, 149, 148, 146, 145, 144, 143, 
  141, 140, 139, 137, 136, 134, 133, 132, 130, 129, 
  127, 125, 124, 122, 121, 119, 117, 116, 114, 112, 
  111, 109, 107, 106, 104, 102, 100, 99, 97, 95, 93, 
  92, 90, 88, 86, 85, 83, 81, 79, 77, 76, 74, 72, 70, 
  69, 67, 65, 63, 62, 60, 58, 57, 55, 53, 52, 50, 48, 
  47, 45, 44, 42, 40, 39, 37, 36, 35, 33, 32, 30, 29, 
  28, 26, 25, 24, 23, 21, 20, 19, 18, 17, 16, 15, 14, 
  13, 12, 11, 10, 9, 8, 8, 7, 6, 5, 5, 4, 4, 3, 3, 2, 
  2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0
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
