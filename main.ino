#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioRecordQueue   queue1;
AudioPlaySdRaw     playRaw;
AudioInputI2SQuad  i2s_quad_in; 
AudioOutputI2SQuad i2s_quad_out;
 
AudioFilterFIR   fir1;

AudioConnection  patchCord1(i2s_quad_in, 0, i2s_quad_out, 0);
AudioConnection  patchCord2(i2s_quad_in, 1, i2s_quad_out, 1);
AudioConnection  patchCord1(i2s_quad_in, 2, i2s_quad_out, 2);
AudioConnection  patchCord2(i2s_quad_in, 3, i2s_quad_out, 3);


AudioControlSGTL5000     sgtl5000_1;

//SD card pins for built in SD card on Teensy
#define SDCARD_CS_PIN    BUILTIN_SDCARD
#define SDCARD_MOSI_PIN  11  // not actually used
#define SDCARD_SCK_PIN   13  // not actually used

// constants for buffer and sampling
#define SAMPLE_FREQ 8000
#define BUF_SIZE 128

// clock frequency required to drive mics
#define  MCLK_FREQ BUF_SIZE * SAMPLE_FREQ

int buf_index;

long time_start = 0;

File frec;

void setup() {
  Serial.println("Hello");

  /*
  // set up 2 MHz clock on pin 9
  pinMode(CLK_PIN, OUTPUT);
  analogWriteFrequency(CLK_PIN, MCLK_FREQ);
  analogWrite(CLK_PIN, 128);
  Serial.println("started output clock");
  */

  
  // Initialize the SD card
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  Serial.println(SDCARD_CS_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here if no SD card, but print a message
    Serial.println("couldn't find SD");
  }
  Serial.println("initialized SD card");
  
  
  // Initialize volume control
  const int myInput = AUDIO_INPUT_MIC;
  Serial.println('0');
  sgtl5000_1.enable();
  Serial.println('1');
  sgtl5000_1.inputSelect(myInput);
  Serial.println('2');
  sgtl5000_1.volume(0.5);
  Serial.println("initialized sgt");
}

void loop() {
  
  if (SD.exists("RECORD.RAW")) {
    // The SD library writes new data to the end of the
    // file, so to start a new recording, the old file
    // must be deleted before new data is written.
    SD.remove("RECORD.RAW");
  }
  frec = SD.open("RECORD.RAW", FILE_WRITE);
  if (frec) {
    Serial.println("Beginning recording");
    i2s_quad_in.begin();
  }
  else {
    Serial.println("could not begin recording");
  }

  time_start = millis();

  //record for 5 seconds
  Serial.println("beginning recording");
  while(millis() - time_start < 5000) {
    if (queue1.available() >= 2) {
      byte buffer[512];
      // Fetch 2 blocks from the audio library and copy
      // into a 512 byte buffer.  The Arduino SD library
      // is most efficient when full 512 byte sector size
      // writes are used.
      memcpy(buffer, queue1.readBuffer(), 256);
      queue1.freeBuffer();
      memcpy(buffer+256, queue1.readBuffer(), 256);
      queue1.freeBuffer();
      // write all 512 bytes to the SD card
      //elapsedMicros usec = 0;
      frec.write(buffer, 512);
      // Uncomment these lines to see how long SD writes
      // are taking.  A pair of audio blocks arrives every
      // 5802 microseconds, so hopefully most of the writes
      // take well under 5802 us.  Some will take more, as
      // the SD library also must write to the FAT tables
      // and the SD card controller manages media erase and
      // wear leveling.  The queue1 object can buffer
      // approximately 301700 us of audio, to allow time
      // for occasional high SD card latency, as long as
      // the average write time is under 5802 us.
      //Serial.print("SD write, us=");
      //Serial.println(usec);
    }
  }

  //stop recording
  Serial.println("stopping recording");
  queue1.end();
  while (queue1.available() > 0) {
    frec.write((byte*)queue1.readBuffer(), 256);
    queue1.freeBuffer();
  }
  frec.close();


  // play 5 second sample
  Serial.println("beginning playing");
  playRaw.play("RECORD.RAW");
  while(playRaw.isPlaying()) {
    Serial.println("playing raw file");
  }
  Serial.println("stopping playing");
  playRaw.stop();
}
