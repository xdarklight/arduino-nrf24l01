/**
 * A Mirf example to test the latency between an
 * Ardunio (running as server) and ATTiny85 (running as client).
 *
 * Pins:
 * Hardware SPI:
 * MISO -> PB0
 * MOSI -> PB1
 * SCK -> PB2
 *
 * (Configurable):
 * CE -> PB4
 * CSN -> PB3
 */

#include <SPI85.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpi85Driver.h>

void setup(){
  /*
   * Setup pins / SPI.
   */

  Mirf.cePin = PB4;
  Mirf.csnPin = PB3;

  Mirf.spi = &MirfHardwareSpi85;
  Mirf.init();
  
  /*
   * Configure reciving address.
   */
   
  Mirf.setRADDR((byte *)"clie1");
  
  /*
   * Set the payload length to sizeof(unsigned long) the
   * return type of millis().
   *
   * NB: payload on client and server must be the same.
   */
   
  Mirf.payload = sizeof(unsigned long);
  
  /*
   * Write channel and payload config then power up reciver.
   */
   
  /*
   * To change channel:
   * 
   * Mirf.channel = 10;
   *
   * NB: Make sure channel is legal in your area.
   */
   
  Mirf.config();
}

void loop() {
  unsigned long time = millis();
  
  Mirf.setTADDR((byte *)"serv1");
  
  Mirf.send((byte *)&time);
  
  while(Mirf.isSending()){
  }

  delay(10);
  while(!Mirf.dataReady()){
    //Serial.println("Waiting");
    if ( ( millis() - time ) > 1000 ) {
      return;
    }
  }
  
  Mirf.getData((byte *) &time);

  delay(1000);
} 
  
  
  
