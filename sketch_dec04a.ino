
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav1;     //xy=140,330
AudioPlaySdWav           playSdWav2;     //xy=152,386
AudioMixer4              mixer1;         //xy=348,362
AudioOutputI2S           i2s1;           //xy=531,367
AudioConnection          patchCord1(playSdWav1, 0, mixer1, 0);
AudioConnection          patchCord2(playSdWav1, 1, mixer1, 1);
AudioConnection          patchCord3(playSdWav2, 0, mixer1, 2);
AudioConnection          patchCord4(playSdWav2, 1, mixer1, 3);
AudioConnection          patchCord5(mixer1, 0, i2s1, 0);
AudioConnection          patchCord6(mixer1, 0, i2s1, 1);
// GUItool: end automatically generated code


AudioControlSGTL5000     sgtl5000_1;

File files[4];

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

void setup() {
  Serial.begin(9600);

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(12);

  // Comment these out if not using the audio adaptor board.
  // This may wait forever if the SDA & SCL pins lack
  // pullup resistors
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  while (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
      Serial.println("Unable to access the SD card");
      delay(500);
  }

  File root = SD.open("/");

  for(int i=0; i<4; i++) 
  {
    File entry = root.openNextFile();
    files[i] = entry;
    Serial.println(entry.name());
  } 

  mixer1.gain(0, .2);
  mixer1.gain(1, .2);
  mixer1.gain(2, .2);
  mixer1.gain(3, .2);
  
}



void playFile(AudioPlaySdWav playWav, const char *filename)
{
  Serial.print("Playing file: ");
  Serial.println(filename);

  // Start playing the file.  This sketch continues to
  // run while the file plays.
  playWav.play(filename);

  // A brief delay for the library read WAV info
  delay(5);

  // Simply wait for the file to finish playing.
  while (playWav.isPlaying()) {
    // uncomment these lines if you audio shield
    // has the optional volume pot soldered
    //float vol = analogRead(15);
    //vol = vol / 1024;
    // sgtl5000_1.volume(vol);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  playFile(playSdWav1, files[1].name());
  delay(500);
  playFile(playSdWav2, files[2].name());
  delay(500);

  Serial.println("playing both");
  playSdWav1.play(files[1].name());
  playSdWav2.play(files[2].name());
  delay(500);
  Serial.println("done");
}
