#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "Wire.h"
#include "Adafruit_BMP085.h"
#include <Filters.h>

Adafruit_BMP085 bmp;

#define ledPin 5

//
// Hardware configuration
//
RF24 radio(9,10);

const uint64_t pipes[2] = { 0xe7e7e7e7e7LL, 0xc2c2c2c2c2LL };

typedef struct{
  int32_t pressure;
  float temperature;
  
} typeData;

typeData data;

FilterTwoPole pressureFilter;

void setup(void)
{
  //
  // Print preamble
  //
  Serial.begin(57600);

  //
  // Setup and configure rf radio
  //
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(true);
  radio.setRetries(15,15);
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.startListening();

  pinMode(ledPin, OUTPUT);
  
  // Jako parametr mozemy podav dokladnosc - domyslnie 3
  // 0 - niski pobór energii - najszybszy pomiar
  // 1 - standardowy pomiar
  // 2 - wysoka precyzja
  // 3 - super wysoka precyzja - najwolniejszy pomiar  
  bmp.begin(2);
  
  pressureFilter.setAsFilter(LOWPASS_BUTTERWORTH, 0.5);
  
}

byte i;
long t;

void loop(void)
{

  t=bmp.readPressure();
  
  pressureFilter.input(t);
  
  i++;
  
  if (i == 50) {
//    Serial.print(t);
//    Serial.print(" ");
//    Serial.println((long) pressureFilter.output());
    i = 0; 
    
    digitalWrite(ledPin, HIGH);
    radio.stopListening();
  
    data.pressure = (int32_t) pressureFilter.output();
    data.temperature = bmp.readTemperature();

    Serial.println(radio.write(&data, sizeof(data)));

    radio.startListening();
    digitalWrite(ledPin, LOW);
    
  }
  
  delay(10);
}


