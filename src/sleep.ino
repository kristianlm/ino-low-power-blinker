
// -*- c -*-
// from http://donalmorrissey.blogspot.no/2010/04/sleeping-arduino-part-5-wake-up-via.html
/*
 * Sketch for testing sleep mode with wake up on WDT.
 * Donal Morrissey - 2011.
 *
 */

#define __AVR_ATmega168__
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

#define LED_PIN (0)

//volatile int f_wdt=1;

typedef unsigned char byte;

/***************************************************
 *  Name:        ISR(WDT_vect)
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Watchdog Interrupt Service. This
 *               is executed when watchdog timed out.
 *
 ***************************************************/
ISR(WDT_vect)
{
  //  if(f_wdt == 0)
    {
      //  f_wdt=1;
    } // else is error!
}

void power_setup();

/***************************************************
 *  Name:        enterSleep
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Enters the arduino into sleep mode.
 *
 ***************************************************/
  /* set new watchdog timeout prescaler value */
  // 6 = 1 sek
  // 7 = 2 sek
  // 10 = 4 sek
  // 11 = 8 sek

void enterSleep(byte prescaler)
{
  //f_wdt  = 0;
    /* Clear the reset flag. */
  MCUSR &= ~(1<<WDRF);

  /* In order to change WDE or the prescaler, we need to
   * set WDCE (This will allow updates for 4 clock cycles).
   */
  WDTCR |= (1<<WDCE) | (1<<WDE);

  WDTCR =
    ((prescaler>>3) & 1)<<WDP3 |
    ((prescaler>>2) & 1)<<WDP2 |
    ((prescaler>>1) & 1)<<WDP1 |
    ((prescaler>>0) & 1)<<WDP0; // page 55

  /* Enable the WD interrupt (note no reset). */
  WDTCR |= _BV(WDIE);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();  // The program will continue from here after the WDT timeout
  sleep_disable();
  power_setup();
}

void power_setup() {
  ADCSRA = 0; // disable ADC (power_
}


void setup()
{
  power_setup();

  for (byte i=0; i<20; i++) {    //make all pins inputs with pullups enabled
    pinMode(i, INPUT_PULLUP);
    digitalWrite(i, HIGH);
  }
  pinMode(LED_PIN, OUTPUT);
}

void shake_blink() {
  for(int i = 0 ; i < 7 ; i++) {
    digitalWrite(LED_PIN, HIGH);
    enterSleep(0); // short burst on
    digitalWrite(LED_PIN, LOW);
    enterSleep(2); // longer delay off
  }
}
void burst_blink() {
  digitalWrite(LED_PIN, HIGH);
  enterSleep(0); // short burst on
  digitalWrite(LED_PIN, LOW);
  enterSleep(5); // longer delay off
}

void blink_signal(char msg) {
  while(--msg >= 0) {
    burst_blink();
  }
}
int counter = 0;
void loop()
{
  // old way of sleeping:
  // volatile byte k = 0;      for(int i = 0 ; i < 2000 ; i++)        k = 1;
  counter++;
  shake_blink();
  enterSleep(6);
  int cr = counter;
  while(cr > 0) {
    blink_signal(1 + (cr % 10));
    enterSleep(7);
    cr /= 10;
  }
}
