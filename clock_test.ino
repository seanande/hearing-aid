#include "kenetis.h"
#include "core_pins.h"

void config_MCLK(void)
{
  SIM_SCGC6 |= SOM_SCGC6_I2S;
  CORE_PIN11_CONFIG = PORT_PCR_MUX6);
  
  #define MCLK_SRC 1 //use 16MHz Xtal clock
  #define MCLK_MULT 24
  #define MCLK_DIV 125 //get 3.072 MHz clock for mics

  I2S0_MDR = I2S_MDR_FRACT((MCLK_MULT-1)) | I2S_MDR_DIVIDE((MCLK_DIV-1));
}

void setup() {
  // put your setup code here, to run once:
  while(!Serial)
  #define CLK_PIN 13 //change later
  pinMode(CLK_PIN, OUTPUT);
  config_MCLK();
}

void loop() {
  // put your main code here, to run repeatedly:

}
