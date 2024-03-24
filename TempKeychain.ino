// required library to use neopixels
#include <Adafruit_NeoPixel.h>

#define LED_PIN 5 // pin for strip
#define NUM_LEDS 8 // number of leds on the strip
#define SENSOR_PIN A3 // sensor pin

// initialize the neopixel strip           vvv these values are related to the type of strip used
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);

/*
   The following text and arrays are the colors for each temperature range.
   Apart from the extremes, each color describes a 2.5ºF temperature range.
   Because the neopixel strip can only display 8 pixels at a time, the colors
   are separated in groups of 8.
*/

// Burning >=100 (flashing Red)

// (inclusive)-(exclusive)
// 80-100
uint32_t  hot[8] = {
  strip.Color(250, 150, 5, 0),
  strip.Color(250, 130, 5, 0),
  strip.Color(250, 110, 5, 0),
  strip.Color(250, 90, 5, 0),
  strip.Color(250, 70, 5, 0),
  strip.Color(250, 50, 5, 0),
  strip.Color(250, 30, 5, 0),
  strip.Color(250, 10, 5, 0)
};

// 60-80
uint32_t  temperate[8] = {
  strip.Color(150, 255, 5, 0),
  strip.Color(120, 255, 5, 0),
  strip.Color(90, 255, 5, 0),
  strip.Color(60, 255, 5, 0),
  strip.Color(30, 255, 5, 0),
  strip.Color(5, 255, 10, 0),
  strip.Color(5, 255, 30, 0),
  strip.Color(5, 255, 60, 0)
};

// 40-60
uint32_t  frigid[8] = {
  strip.Color(245, 100, 245, 0),
  strip.Color(215, 100, 245, 0),
  strip.Color(185, 100, 245, 0),
  strip.Color(155, 100, 245, 0),
  strip.Color(125, 100, 245, 0),
  strip.Color(100, 105, 245, 0),
  strip.Color(100, 135, 245, 0),
  strip.Color(100, 165, 245, 0)
};

// 20-40
uint32_t  cool[8] = {
  strip.Color(66, 245, 245, 0),
  strip.Color(66, 227, 245, 0),
  strip.Color(66, 206, 245, 0),
  strip.Color(66, 194, 245, 0),
  strip.Color(66, 173, 245, 0),
  strip.Color(66, 149, 245, 0),
  strip.Color(66, 120, 245, 0),
  strip.Color(66, 90, 245, 0)
};

//  0-20
uint32_t  freezing[8] = {
  strip.Color(5, 245, 245, 0),
  strip.Color(40, 245, 245, 0),
  strip.Color(75, 245, 245, 0),
  strip.Color(110, 245, 245, 0),
  strip.Color(145, 245, 245, 0),
  strip.Color(180, 245, 245, 0),
  strip.Color(215, 245, 245, 0),
  strip.Color(250, 245, 245, 0)
};

// frostbite <0 (flashing white)

void setup() {
  pinMode(SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  // put your setup code here, to run once:
  // begin Serial - prints out temperature value
  Serial.begin(9600);
  // begin neopixel strip
  strip.begin();
  strip.setBrightness(20); // remove later or change for final product
  // show strip to clear out all of the pixels
  strip.show();

  // display the life of the battery
  // analogReference(INTERNAL); at the end of the function
  displayBatteryLife();

}

// control when the next time is to switch
unsigned long switchTime = 0;
// how long the gauge gets displayed for
// animations are displayed using gaugeToAnimationRatio
const unsigned long switchInterval = 20000; // 20 seconds
// animation mode variable
bool animation = true;
// modify delay in taking temperature values
// longer delay values allow the animations to go on for longer
// this is really only important in the animations that store values like the fades
const float delayMultiplier = 1.0f;
// the ratio of gauge display time to animation display time (approximatley)
const float gaugeToAnimationRatio = 4.0f;

void loop() {
  // check if it is time to switch animation mode
  if (millis() > switchTime) {
    switchAnimation();
  }
 
  // display temperature
  displayTemperature(readTemp(), animation);

  // no delay needed because delay is built into the displayTemperature() method
}


/**
 * switch animation mode
 */
void switchAnimation() {
  // switch
  animation = !animation;
  // set next switch time
  if (animation) {
    // lower time for animation
    switchTime = millis() + (switchInterval / gaugeToAnimationRatio);
  } else {
    switchTime = millis() + switchInterval;
  }
}

void displayBatteryLife(){
  analogReference(DEFAULT);
  delay(500);
  // clear LED strip of any effects
  strip.clear();
  // get average vcc
  long totalVcc = 0;
  for (int i = 0; i < 8; i++) {
    // display a progress bar on the strip
    strip.setPixelColor(i, 240, 96, 230, 0); // magenta
    strip.show();
    delay(100);
    // add vcc value
    totalVcc+= readVcc();
  }
  // divide by number of data values & turn into volts instead of millivolts
  float vcc = (float) totalVcc / (8 * 1000);
  Serial.print("Vcc: "); Serial.println(vcc);
  //printValue(vcc);
  // 5 - full
  // 2.7 - empty
  float full = 5.0;
  float empty = 2.7;
  float percent = (vcc - empty) / (full - empty);
  Serial.print(percent * 100); Serial.println("%");
  // display percent
  strip.clear();
  // led to stop and blink on
  int stopLED = 8 * percent;
  for (int i = 0; i < 8; i++) {
    if (i < stopLED) {
      // light up led
      strip.setPixelColor(i, 90, 232, 105, 0); // lime
      strip.show();
      delay(500);
    } else if (i == stopLED) {
      // blink 3 times
      for (int j = 0; j < 3; j++) {
        strip.setPixelColor(i, 90, 232, 105, 0);
        strip.show();
        delay(500);
        strip.setPixelColor(i, 0, 0, 0, 0);
        strip.show();
        delay(500);
      }
    } else {
      break;
    }
  }
  if (stopLED < 0) {
    // blink first led red 3 times
    for (int j = 0; j < 3; j++) {
        strip.setPixelColor(0, 255, 0, 0, 0);
        strip.show();
        delay(500);
        strip.setPixelColor(0, 0, 0, 0, 0);
        strip.show();
        delay(500);
      }
  }
  analogReference(INTERNAL);
  delay(10);
}

long readVcc() {
 
long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

void printValue(float value) {
  //temporarily holds data from vals
  char charVal[8] = {'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'};

  //4 is mininum width, 3 is precision; float value is copied onto buff
  dtostrf(value, 4, 3, charVal);
  Serial.print("Printing value: "); Serial.println(charVal);
  strip.clear();
  for (int i = 0; i < 8; i++) {
    if (charVal[i] == 'x') {
      break;
    }
    if (charVal[i] == '0') {
      strip.setPixelColor(i, 255, 255, 0, 0);
        strip.show();
        delay(1000);
        strip.setPixelColor(i, 0, 0, 0, 0);
        strip.show();
        delay(1000);
    }
    if (charVal[i] == '.') {
      // decimal
      for (int j = 0; j < 3; j++) {
        strip.setPixelColor(i, 0, 0, 255, 0);
        strip.show();
        delay(500);
        strip.setPixelColor(i, 0, 0, 0, 0);
        strip.show();
        delay(500);
      }
    } else {
      // number
      int digit = charVal[i] - '0';
      for (int j = 0; j < digit; j++) {
        strip.setPixelColor(i, 0, 255, 0, 0);
        strip.show();
        delay(500);
        strip.setPixelColor(i, 0, 0, 0, 0);
        strip.show();
        delay(500);
      }
    }
    delay(1000);
  }
}

/**
   Reads the temperature from the TMP36 sensor and returns the value in ºF
*/
int readTemp() {
  // put your main code here, to run repeatedly:
  //getting the voltage reading from the temperature sensor
  int reading = analogRead(SENSOR_PIN);
  // converting that reading to voltage, for 3.3v arduino use 3.3

  float voltage = reading * 1.1;
  voltage /= 1024.0;

  // now print out the temperature
  float temperatureC = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree wit 500 mV offset
  //to degrees ((voltage - 500mV) times 100)

  //Serial.print(temperatureC); Serial.println(" degrees C");

  // now convert to Fahrenheit
  float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
  Serial.print(temperatureF); Serial.println(" degrees F");
  return temperatureF;
}

/**
   Displays the temperature in ºF on the neopixel stick
   The display can either be a gauge with the 8 pixels, or an animation
*/
void displayTemperature(int temp, bool animation) {
  if (animation) {
    // animations for different temperatures
    if (temp >= 100) {
      // burning
      bounceFade(40, 100, 2 * delayMultiplier);
    } else if (temp >= 80) {
      // hot
      sparkleFade(20, 50, 70 * delayMultiplier);
    } else if (temp >= 60) {
      // temperate
      rainbowCycle(20);
    } else if (temp >= 40) {
      // frigid
      pulse(148, 237, 88);
    } else if (temp >= 20) {
      // cool
      shakingIce();
    } else if (temp >= 0) {
      // freezing
      for (int i = 0; i < 10 * delayMultiplier; i++) {
        SnowSparkle(0x10, 0x10, 0x10, 20, 200);
      }
    } else {
      // below 0
      for (int i = 0; i < 20 * delayMultiplier; i++) {
        SnowSparkleWhite(0x10, 0x10, 0x10, 20, 100);
      }
    }
  } else {
    // display gauge
    // other than the end cases, tempurature is mapped from the value to an index of 0-7
    if (temp >= 100) {
      flashRed();
      delay(1000);
    } else if (temp >= 80) {
      int index = (temp - 80) / 2.5;
      displayGauge(hot, index, 0);
    } else if (temp >= 60) {
      int index = (temp - 60) / 2.5;
      displayGauge(temperate, index, 0);
    } else if (temp >= 40) {
      int index = (temp - 40) / 2.5;
      displayGauge(frigid, index, 0);
    } else if (temp >= 20) {
      int index = (temp - 20) / 2.5;
      displayGauge(cool, index, 0);
    } else if (temp >= 0) {
      int index = temp / 2.5;
      displayGauge(freezing, index, 1);
    } else {
      // below 0
      flashWhite();
      delay(1000);
    }
  }
}

/**
   Display a gauge on the NeoPixels with specified colors
   The stopIndex is the last displayed color on the gauge
   The displayType is how the pixels get displayed
*/
void displayGauge(uint32_t colors[8], int stopIndex, int displayType) {
  if (displayType == 1) {
    // sparkle display
    for (int i = 0; i < NUM_LEDS; i++) {
      SnowSparkleColor(colors[i], 20, 200, stopIndex);
    }
  } else {
    // change all LED colors
    for (int i = 0; i < NUM_LEDS; i++) {
      if (i <= stopIndex) {
        // display color
        strip.setPixelColor(i, colors[i]);

      } else {
        // turn off
        strip.setPixelColor(i, 0, 0, 0, 0);
      }
    }
    strip.show();
    delay(500);
  }
}

// flash all of the LEDs on the neopixel white
void flashWhite() {
  for (int j = 0; j < NUM_LEDS; j++) {
    strip.setPixelColor(j, 0, 0, 0, 255);
  }
  strip.show();
  delay(50);
  for (int j = 0; j < NUM_LEDS; j++) {
    strip.setPixelColor(j, 0, 0, 0, 0);
  }
  strip.show();
  delay(50);
}

// Flash all of the LEDs omn the neopixel red (and a little white too)
void flashRed() {
  for (int j = 0; j < NUM_LEDS; j++) {
    strip.setPixelColor(j, 255, 0, 0, 100);
  }
  strip.show();
  delay(50);
  for (int j = 0; j < NUM_LEDS; j++) {
    strip.setPixelColor(j, 0, 0, 0, 0);
  }
  strip.show();
  delay(50);
}

/**
   Turn on random pixels, and fade them
*/
void sparkleFade(int fadeMultiplier, int delayAmount, int repetitions) {
  // this is the base color to sparkle
  int r = 255;
  int g = 80;
  int b = 0;
  int fadeAmount[NUM_LEDS]; // this controls how much each led is faded
  // initialize fade amount
  for (int i = 0; i < NUM_LEDS; i++) {
    fadeAmount[i] = 300 / fadeMultiplier; // initialized at max fade amount (LED off)
  }
  // repeat the number of repetitions
  for (int k = 0; k < repetitions; k++) {
    // pick random LED
    int randomLED = random(NUM_LEDS);
    // set fade to 0 of random LED
    fadeAmount[randomLED] = 0;
    // change color of all leds with the correct fade amount
    for (int j = 0; j < NUM_LEDS; j++) {
      // get fade amounts
      int r1 = r - (fadeAmount[j] * fadeMultiplier);
      int g1 = g - (fadeAmount[j] * fadeMultiplier);
      int b1 = b - (fadeAmount[j] * fadeMultiplier);
      // make sure none are below zero (this will cause colors we don't want)
      if (r1 < 0) {
        r1 = 0;
      }
      if (b1 < 0) {
        b1 = 0;
      }
      if (g1 < 0) {
        g1 = 0;
      }
      // set color on strip ready to be displayed
      strip.setPixelColor(j, r1, g1, b1, 0);
      // increment fade amount, so the next time this loop is run, it will be slightly dimmer
      fadeAmount[j]++;
    }
    // show the colors on the strip
    strip.show();
    delay(delayAmount);
  }
}

/**
   Similar to the sparkleFade() method,
   the bounceFade will just go back and forth across the strip
*/

void bounceFade(int fadeMultiplier, int delayAmount, int repetitions) {
  // base LED color
  int r = 255;
  int g = 80;
  int b = 0;
  int fadeAmount[NUM_LEDS]; // this controls how much each led is faded
  // initialize fade amount
  for (int i = 0; i < NUM_LEDS; i++) {
    fadeAmount[i] = 300 / fadeMultiplier; // initialize at max fade amount (LED off)
  }
  // repeat repetitions amount of times
  for (int k = 0; k < repetitions; k++) {
    // i is the index of the brightest LED
    // LEDs below the index fade away
    // this loop is for the LED movement forward
    for (int i = 0; i < NUM_LEDS; i++) {
      // remove fade for i led
      fadeAmount[i] = 0;
      // update all LED values
      for (int j = 0; j < NUM_LEDS; j++) {
        int r1 = r - (fadeAmount[j] * fadeMultiplier);
        int g1 = g - (fadeAmount[j] * fadeMultiplier);
        int b1 = b - (fadeAmount[j] * fadeMultiplier);
        // make sure values aren't below 0
        if (r1 < 0) {
          r1 = 0;
        }
        if (b1 < 0) {
          b1 = 0;
        }
        if (g1 < 0) {
          g1 = 0;
        }
        strip.setPixelColor(j, r1, g1, b1, 0);
        fadeAmount[j]++; // increment fade amount
      }
      strip.show();
      delay(delayAmount);
    }

    // go in opposite direction
    for (int i = NUM_LEDS - 1; i >= 0; i--) {
      // update all LED values
      fadeAmount[i] = 0;
      for (int j = 0; j < NUM_LEDS; j++) {
        int r1 = r - (fadeAmount[j] * fadeMultiplier);
        int g1 = g - (fadeAmount[j] * fadeMultiplier);
        int b1 = b - (fadeAmount[j] * fadeMultiplier);
        // make sure values aren't below 0
        if (r1 < 0) {
          r1 = 0;
        }
        if (b1 < 0) {
          b1 = 0;
        }
        if (g1 < 0) {
          g1 = 0;
        }
        strip.setPixelColor(j, r1, g1, b1, 0);
        fadeAmount[j]++;
      }
      strip.show();
      delay(delayAmount);
    }
  }
}

/**
   This method shakes the color back and forth
*/
void shakingIce() {
  int r = 52;
  int g = 229;
  int b = 235;
  int nonIceDivisor = 30;
  int shakeAmount = 2;
  for (int i = 0; i < shakeAmount; i++) {
    strip.clear();
    // center LEDs are ice
    strip.setPixelColor(0, r / nonIceDivisor, g / nonIceDivisor, b / nonIceDivisor, 0);
    strip.setPixelColor(NUM_LEDS - 1, r / nonIceDivisor, g / nonIceDivisor, b / nonIceDivisor, 0);
    for (int i = 1; i < NUM_LEDS - 1; i++) {
      strip.setPixelColor(i, r, g, b, 30);
    }
    strip.show();
    delay(1000);

    // shake left
    strip.setPixelColor(NUM_LEDS - 2, r / nonIceDivisor, g / nonIceDivisor, b / nonIceDivisor, 0);
    strip.setPixelColor(0, r, g, b, 30);
    strip.show();
    delay(200);
    // shake middle
    strip.setPixelColor(NUM_LEDS - 2, r, g, b, 30);
    strip.setPixelColor(0,  r / nonIceDivisor, g / nonIceDivisor, b / nonIceDivisor, 0);
    strip.show();
    delay(200);
    // shake right
    strip.setPixelColor(0, r / nonIceDivisor, g / nonIceDivisor, b / nonIceDivisor, 0);
    strip.setPixelColor(1, r / nonIceDivisor, g / nonIceDivisor, b / nonIceDivisor, 0);
    strip.setPixelColor(NUM_LEDS - 1, r, g, b, 30);
    strip.setPixelColor(NUM_LEDS - 2, r, g, b, 30);
    strip.show();
    delay(400);
  }
}

/*
   This method fades in an out a color
*/
void pulse(int r, int g, int b) {
  for (int i = 0; i < 255; i++) {
    // could be r - 255 + i
    int red = r - (255 - i);
    int green = g - (255 - i);
    int blue = b - (255 - i);
    if (red < 0) {
      red = 0;
    }
    if (green < 0) {
      green = 0;
    }
    if (blue < 0) {
      blue = 0;
    }
    for (int j = 0; j < NUM_LEDS; j++) {
      strip.setPixelColor(j, red, green, blue, 0);
    }
    strip.show();
    delay(5);
  }
  for (int i = 0; i < 255; i++) {
    int red = r - (i);
    int green = g - (i);
    int blue = b - (i);
    if (red < 0) {
      red = 0;
    }
    if (green < 0) {
      green = 0;
    }
    if (blue < 0) {
      blue = 0;
    }
    for (int j = 0; j < NUM_LEDS; j++) {
      strip.setPixelColor(j, red, green, blue, 0);
    }
    strip.show();
    delay(5);
  }
  strip.clear();
  strip.show();
}

/**
   rainbowCycle(), Wheel(), and SnowSparkle() functions come from the site:
   https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
*/

void rainbowCycle(int SpeedDelay) {
  byte *c;
  uint16_t i, j;

  for (j = 0; j < 256 * delayMultiplier; j++) { // 1 * delayMultiplier cycles of all colors on wheel
    for (i = 0; i < NUM_LEDS; i++) {
      c = Wheel(((i * 256 / NUM_LEDS) + j) & 255);
      strip.setPixelColor(i, *c, *(c + 1), *(c + 2), 0);
    }
    strip.show();
    delay(SpeedDelay);
  }
}

byte * Wheel(byte WheelPos) {
  static byte c[3];

  if (WheelPos < 85) {
    c[0] = WheelPos * 3;
    c[1] = 255 - WheelPos * 3;
    c[2] = 0;
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    c[0] = 255 - WheelPos * 3;
    c[1] = 0;
    c[2] = WheelPos * 3;
  } else {
    WheelPos -= 170;
    c[0] = 0;
    c[1] = WheelPos * 3;
    c[2] = 255 - WheelPos * 3;
  }

  return c;
}

void SnowSparkle(byte red, byte green, byte blue, int SparkleDelay, int SpeedDelay) {
  for (int j = 0; j < NUM_LEDS; j++) {
    strip.setPixelColor(j, red, green, blue, 0);
  }

  int Pixel = random(NUM_LEDS);
  strip.setPixelColor(Pixel, 0xff, 0xff, 0xff, 0);
  strip.show();
  delay(SparkleDelay);
  strip.setPixelColor(Pixel, red, green, blue, 0);
  strip.show();
  delay(SpeedDelay);
}

/**
   A copy of SnowSparkle, but using a neopixel color instead of 3 bytes
   and the number of leds to affect
*/

void SnowSparkleColor(uint32_t color, int SparkleDelay, int SpeedDelay, int leds) {
  for (int j = 0; j < leds; j++) {
    strip.setPixelColor(j, color);
  }

  int Pixel = random(leds);
  strip.setPixelColor(Pixel, 0xff, 0xff, 0xff, 0);
  strip.show();
  delay(SparkleDelay);
  strip.setPixelColor(Pixel, color);
  strip.show();
  delay(SpeedDelay);
}

/**
   A copy of SnowSparkle() but using the white led
*/
void SnowSparkleWhite(byte red, byte green, byte blue, int SparkleDelay, int SpeedDelay) {
  for (int j = 0; j < NUM_LEDS; j++) {
    strip.setPixelColor(j, red, green, blue, 200);
  }

  int Pixel = random(NUM_LEDS);
  strip.setPixelColor(Pixel, 0xff, 0xff, 0xff, 0);
  strip.show();
  delay(SparkleDelay);
  strip.setPixelColor(Pixel, red, green, blue, 200);
  strip.show();
  delay(SpeedDelay);
}
