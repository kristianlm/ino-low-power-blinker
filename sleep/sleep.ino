
// -*- c -*-
// from http://donalmorrissey.blogspot.no/2010/04/sleeping-arduino-part-5-wake-up-via.html
/*
 * Sketch for testing sleep mode with wake up on WDT.
 * Donal Morrissey - 2011.
 *
 */

#define __AVR_ATtiny85__
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <wiring_private.h>

#define LED_PIN (0)
#define SPEAKER_PIN (1) // you can't really change this, it's connected to the PWM

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

void enterSleep(byte prescaler) {
  //f_wdt  = 0;
    /* Clear the reset flag. */
  MCUSR &= ~(1<<WDRF);

  /* In order to change WDE or the prescaler, we need to
   * set WDCE (This will allow updates for 4 clock cycles). Page 55.
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
  sleep_mode();  // The program will continue from here after the WDT timeout

  // magic procedure to disable the watchdog timer. see 8.4.2.
  MCUSR = 0x00; // clear all reset flags (I don't think you need this)
  WDTCR |= (1<<WDCE) | (1<<WDE); // turn on "Watchdog Change Enable"
  WDTCR = 0x00; // to allow turning the Watchdog off

  // I couldn't get this to work. My chip reports the BODSE bit is
  // zero, even right after setting it so I think it's we've met the
  // "limitations" described in the manual.
  // disable Brown-out detection during sleep (it consumes a lot of power)
  // page 37: First, both BODS and BODSE must be set to one. Second,
  // within four clock cycles, BODS must be set to one and BODSE must
  // be set to zero
  // MCUCR |= (1<<BODS) | (1<<BODSE);
  // MCUCR |= (1<<BODS);
  // MCUCR &= ~(1<<BODSE);

  power_setup();
}

void power_setup() {
  // this should reduce sleep modes from 0.2mA to 0.02mA
  //ADCSRA = 0; // disable ADC power
}


void setup()
{
  power_setup();

  for (byte i=0; i<20; i++) {    //make all pins inputs with pullups enabled
    pinMode(i, INPUT_PULLUP);
    digitalWrite(i, LOW);
  }
  pinMode(LED_PIN, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void shake_blink() {
  for(int i = 0 ; i < 7 ; i++) {
    digitalWrite(LED_PIN, HIGH);
    enterSleep(0); // short burst on
    digitalWrite(LED_PIN, LOW);
    enterSleep(0); // longer delay off
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

// obs: Arduino's built-in delay relies on the prescaler, so it won't
// get the timings right.
void pwm_prescaler(byte prescaler) {
  if(prescaler & 0b100)  sbi(TCCR0B, CS02);
  else                   cbi(TCCR0B, CS02);
  if(prescaler & 0b010)  sbi(TCCR0B, CS01);
  else                   cbi(TCCR0B, CS01);
  if(prescaler & 0b001)  sbi(TCCR0B, CS00);
  else                   cbi(TCCR0B, CS00);
}

#define LEDON  (PORTB |= _BV(PORTB0)); //digitalWrite(LED_PIN, 1);
#define LEDOFF (PORTB &= ~_BV(PORTB0)); //digitalWrite(LED_PIN, 0);
#define PAUSE  __asm__("nop\n nop\n nop\n nop\n");

void mypwm(byte octave) {
  volatile byte k = 0;
  int cyc = (6 / octave) + 1;
  for(int m = 0 ; m < 200 ; m++) {

    if(octave == 3) {
      LEDON PAUSE PAUSE LEDOFF PAUSE PAUSE;
      LEDON PAUSE PAUSE LEDOFF PAUSE PAUSE;
      LEDON PAUSE PAUSE LEDOFF PAUSE PAUSE;
    }
    if(octave == 2) {
      LEDON PAUSE PAUSE PAUSE LEDOFF PAUSE PAUSE PAUSE;
      LEDON PAUSE PAUSE PAUSE LEDOFF PAUSE PAUSE PAUSE;
    }
    /* for(int j = 0 ; j < octave ; j++) { */
    /*   digitalWrite(LED_PIN, 1); */
    /*   for(int i = 0 ; i < cyc ; i++) k = 1; */
    /*   //__asm__("nop"); */
    /*   digitalWrite(LED_PIN, 0); */
    /*   //__asm__("nop"); */
    /*   for(int i = 0 ; i < cyc ; i++) k = 1; */
    /* } */
  }
}

const int calibration = -3015; // you'll never guess how I derived this

void bang_bit(byte bit) {
  volatile byte k = 0;
  int i = 0;
  int wait_cycles = 10000;
  if(bit) { // mark (high freq)
    for(int j = 0 ; j < 78 ; j++) {
      for(int i = 0 ; i < 5 ; i++) k = 1;
      digitalWrite(LED_PIN, 1);
      for(int i = 0 ; i < 5 ; i++) k = 1;
      digitalWrite(LED_PIN, 0);
    }
    //pwm_prescaler(0b001);
    //mypwm(3);
    //for( ; i < wait_cycles * 1 ; i++) k = 1;
  } else { // space (low freq)

    for(int j = 0 ; j < 40 ; j++) {
      for(int i = 0 ; i < 10 ; i++) k = 1;
      digitalWrite(LED_PIN, 1);
      for(int i = 0 ; i < 10 ; i++) k = 1;
      digitalWrite(LED_PIN, 0);
    }

    //pwm_prescaler(0b011);
    //mypwm(2);
    //for( ; i < (wait_cycles * 2) + calibration ; i++)      k = 1;
  }
}



#define TONE    analogWrite(SPEAKER_PIN, 0x80);
#define SILENCE analogWrite(SPEAKER_PIN, 0x00);

#define DITLEN 60
#define SLP(dits) delay(DITLEN * dits);

void dit() {
  TONE; SLP(1); SILENCE; SLP(1);
}

void dah() {
  TONE; SLP(3); SILENCE; SLP(1);
}



const char alphabet [] PROGMEM =
  "A.-####"
  "B-...##"
  "C-.-.##"
  "D-..###"
  "E.#####"
  "F..-.##"
  "G--.###"
  "H....##"
  "I..####"
  "J.---##"
  "K.-.-##"
  "L.-..##"
  "M--####"
  "N-.####"
  "O---###"
  "P.--.##"
  "Q--.-##"
  "R.-.###"
  "S...###"
  "T-#####"
  "U..-###"
  "V...-##"
  "W.--###"
  "X-..-##"
  "Y-.--##"
  "Z--..##"
  "       " // 7 units
  "1.----#"
  "2..---#"
  "3...--#"
  "4....-#"
  "5.....#"
  "6-....#"
  "7--...#"
  "8---..#"
  "9----.#"
  "0-----#"
  "..-.-.-"
  ",--..--"
  "?..--.."
  "!-.-.--"
  ":---..."
  ";-.-.-."
  "(-.--.#"
  ")-.--.-"
  "@.--.-."
  "&.-...#"
 "\".-..-.";

// fills codedest with a string like "--.", for example.
// returns the length of that string (which is also null-terminated)
// codedest must be large enough for morecode (6 bytes it seems)
int aref(char letter, char* codedest) {

  for(int i = 0 ; i < sizeof(alphabet) ; i++) {
    char a = pgm_read_byte(&(alphabet[i]));

    if(a == letter) {
      if(codedest)
        memcpy_P(codedest, &alphabet[i + 1], 6);
      int len = 0;
      while(codedest[len] != '#' && len < 6) len++;
      codedest[len] = '\x00';
      return len;
    }
    // skip code part (always 6 bytes)
    i+=6;
  }
  return 0;
}

void punch1(char letter) {
  char morse [7];
  int len;
  if(len = aref(letter, morse)) {
    for(int i = 0 ; i < len ; i++) {
      if(morse[i] == '.')      { dit();  }
      else if(morse[i] == '-') { dah();  }
      else if(morse[i] == ' ') { SLP(1); }
      // some error signal
      else {TONE ; delay(10); SILENCE; delay(10); TONE ; delay(10); SILENCE ; delay(10); }
    }
  }
}

void punch(char *txt, int lms) {
  int len = strlen(txt);
  for(int i = 0 ; i < len ; i++) {
    punch1(txt[i]);
    delay(lms);
  }
}

void loop2() {
  //return;
  punch("HEI INGUNN", 5000);
  return;
  const char check = 0;//pgm_read_byte(&(alphabet[0]));
  if(check == 'A') dah();
  else dit();
  return;
  //dah(); dah(); dit();
  //morseout("A");
  //TONE; SLP(1); SILENCE; SLP(1);
  //delay(10000);
  enterSleep(8);
}

// Notes
const int Note_C  = 239;
const int Note_CS = 225;
const int Note_D  = 213;
const int Note_DS = 201;
const int Note_E  = 190;
const int Note_F  = 179;
const int Note_FS = 169;
const int Note_G  = 159;
const int Note_GS = 150;
const int Note_A  = 142;
const int Note_AS = 134;
const int Note_B  = 127;

void TinyTone(unsigned char divisor, unsigned char octave, unsigned long duration) {
  TCCR1 = 0x90 | (8-octave); // for 1MHz clock
  // TCCR1 = 0x90 | (11-octave); // for 8MHz clock
  OCR1C = divisor-1;         // set the OCR
  delay(duration);
  TCCR1 = 0x90;              // stop the counter
}

#define MAGMS 4 // magnitude duration in milliseconds
#define BITLENGTH (MAGMS * 6) // duration of a bit in milliseconds

void space() {
  TinyTone(Note_D, 7, BITLENGTH);
}
void mark() {
  TinyTone(Note_G, 7, BITLENGTH);
}

void bitbang1(byte value) {
  mark(); // carrier
  space(); // start bit
  for(int i = 0 ; i < 8 ; i++) {
    if( value & 0x01) mark();
    else space();
    value = value >> 1;
  }
  mark(); mark(); mark(); // stop bit + carrier
}

void bitbang(char *txt, int len) {
  for(int i = 0 ; i < len ; i++) {
    bitbang1(txt[i]);
    delay(16);
  }
}

int count = 0;
void loop() {
  count++;
  digitalWrite(LED_PIN, LOW);
  char txt[] = "             Hello world!\n";
  sprintf(txt, "KLM\n", count);
  bitbang(txt, strlen(txt));

  pinMode(A2, INPUT);
  digitalWrite(A2, LOW);
  analogReference(INTERNAL);
  while(1) {
    sprintf(txt, "%d\n",
            (int)((100.0*1.1*(analogRead(A2) / 1023.0) - 50) * 10));
    bitbang(txt, strlen(txt));
    enterSleep(6);
  }

  pinMode(SPEAKER_PIN, INPUT);
  pinMode(LED_PIN, INPUT);
  enterSleep(2);
  //  enterSleep(8);
  //enterSleep(8);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
}
