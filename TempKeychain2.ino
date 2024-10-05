/*
* This code is modified to work with an Adafruit Trinket M0 and 16 LEDs
*/

// required library to use neopixels
#include <Adafruit_NeoPixel.h>
#include <Adafruit_DotStar.h>

Adafruit_DotStar internalLED = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);

#define LED_PIN 3 // pin for strip
#define NUM_LEDS 16 // number of leds on the strip
#define SENSOR_PIN A2 // sensor pin

// battery indicator settings
// coincells - 3.0-5.0
// lithium ion - 1.0-3.2
const float maxBatVolt = 3.2;//5.0;
const float minBatVolt = 2;//1.8;
// whether to print the voltage on startup or not
const bool printVoltage = true;

// initialize the neopixel strip           vvv these values are related to the type of strip used
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);

/*
   The following text and arrays are the colors for each temperature range.
   Apart from the extremes, each color describes a 2.5ºF temperature range.
   Because the neopixel strip can only display 8 pixels at a time, the colors
   are separated in groups of 8.
*/

// Burning >=124 (flashing Red)

// (inclusive)-(exclusive)

// 92-124
uint32_t  hot[16] = {
  strip.Color(235, 25, 25, 0),
  strip.Color(236, 53, 16, 0),
  strip.Color(237, 71, 4, 0),
  strip.Color(237, 87, 0, 0),
  strip.Color(237, 101, 0, 0),
  strip.Color(236, 113, 0, 0),
  strip.Color(235, 125, 0, 0),
  strip.Color(234, 137, 0, 0),
  strip.Color(232, 148, 0, 0),
  strip.Color(230, 159, 0, 0),
  strip.Color(228, 169, 0, 0),
  strip.Color(225, 179, 0, 0),
  strip.Color(223, 189, 0, 0),
  strip.Color(220, 198, 22, 0),
  strip.Color(217, 208, 38, 0),
  strip.Color(214, 217, 53, 0)
};

// 60-92
uint32_t  temperate[16] = {
  strip.Color(107, 125, 56, 0),
  strip.Color(102, 124, 57, 0),
  strip.Color(97, 124, 58, 0),
  strip.Color(92, 123, 59, 0),
  strip.Color(87, 123, 60, 0),
  strip.Color(82, 122, 62, 0),
  strip.Color(77, 121, 63, 0),
  strip.Color(72, 120, 65, 0),
  strip.Color(67, 120, 67, 0),
  strip.Color(62, 119, 69, 0),
  strip.Color(57, 118, 71, 0),
  strip.Color(51, 117, 72, 0),
  strip.Color(46, 116, 74, 0),
  strip.Color(40, 115, 76, 0),
  strip.Color(33, 114, 78, 0),
  strip.Color(25, 113, 80, 0)
};

// 28-60
uint32_t  frigid[16] = {
  strip.Color(54, 207, 217, 0),
  strip.Color(0, 201, 223, 0),
  strip.Color(0, 194, 227, 0),
  strip.Color(0, 186, 228, 0),
  strip.Color(0, 178, 227, 0),
  strip.Color(0, 170, 226, 0),
  strip.Color(0, 163, 225, 0),
  strip.Color(0, 155, 224, 0),
  strip.Color(0, 148, 223, 0),
  strip.Color(0, 140, 225, 0),
  strip.Color(0, 132, 231, 0),
  strip.Color(0, 119, 247, 0),
  strip.Color(0, 99, 255, 0),
  strip.Color(71, 82, 255, 0),
  strip.Color(105, 61, 249, 0),
  strip.Color(130, 25, 235, 0)
};

//  (-4)-28
uint32_t  freezing[16] = {
  strip.Color(200, 245, 245, 150),
  strip.Color(200, 245, 245, 150),
  strip.Color(200, 245, 245, 150),
  strip.Color(200, 245, 245, 150),
  strip.Color(200, 245, 245, 150),
  strip.Color(200, 245, 245, 150),
  strip.Color(200, 245, 245, 150),
  strip.Color(200, 245, 245, 150),
  strip.Color(200, 245, 245, 150),
  strip.Color(200, 245, 245, 150),
  strip.Color(200, 245, 245, 150),
  strip.Color(200, 245, 245, 150),
  strip.Color(200, 245, 245, 150),
  strip.Color(200, 245, 245, 150),
  strip.Color(200, 245, 245, 150),
  strip.Color(200, 245, 245, 150)
};

// frostbite <-4 (flashing white)

void setup() {
  pinMode(SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  // put your setup code here, to run once:
  // begin Serial - prints out temperature value
  Serial.begin(9600);
  internalLED.begin();
  internalLED.setPixelColor(0, 64, 64, 0); internalLED.show(); delay(300); 
  internalLED.setPixelColor(0, 0, 64, 46); internalLED.show(); delay(300); 
  internalLED.setPixelColor(0, 64, 0, 64); internalLED.show(); delay(300); 
  internalLED.setPixelColor(0,0,0,0); internalLED.show();
  // begin neopixel strip
  strip.begin();
  strip.setBrightness(20); // remove later or change for final product
  // show strip to clear out all of the pixels
  strip.show();

  // dummy readings
  analogRead(SENSOR_PIN);
  delay(10);
  analogRead(SENSOR_PIN);

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
// this function prints out a numerical value on the NeoPixels
// used for debugging with the battery
void printValue(float value) {
  //temporarily holds data from vals
  char charVal[8] = {'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'};

  //4 is mininum width, 3 is precision; float value is copied onto charVal
  sprintf(charVal,"%f", value);
  // print same value to the serial port (to test if they match)
  Serial.print("Printing value: "); Serial.println(charVal);
  strip.clear();
  // print out the charVal array
  for (int i = 0; i < 8; i++) {
    if (charVal[i] == 'x') {
      // no more numbers
      break;
    }
    if (charVal[i] == '0') {
        // one yellow blink for zero
        strip.setPixelColor(i, 255, 255, 0, 0);
        strip.show();
        delay(1000);
        strip.setPixelColor(i, 0, 0, 0, 0);
        strip.show();
        delay(1000);
    }
    if (charVal[i] == '.') {
      // decimal - 3 blue blinks
      for (int j = 0; j < 3; j++) {
        strip.setPixelColor(i, 0, 0, 255, 0);
        strip.show();
        delay(500);
        strip.setPixelColor(i, 0, 0, 0, 0);
        strip.show();
        delay(500);
      }
    } else {
      // number - blink green the number value
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

  float voltage = reading * 3.3;
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
   The display can either be a gauge with the 16 pixels, or an animation
*/
void displayTemperature(int temp, bool animation) {
  if (animation) {
    // animations for different temperatures
    if (temp >= 124) {
      // burning
      SnowSparkleColor(hot[0], 20, 200, 8);
    } else if (temp >= 92) {
      // hot
      sparkleFade(20, 50, 70 * delayMultiplier);
    }else if (temp >= 60) {
      // temperate
      rainbowCycle(20);
    } else if (temp >= 28) {
      // frigid
      pulse(148, 237, 88);
    }else if (temp >= -4) {
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
    // other than the end cases, tempurature is mapped from the value to an index of 0-15
    if (temp >= 124) {
      flashRed();
      delay(1000);
    } else if (temp >= 92) {
      int index = (temp - 92) / 2;
      displayGauge(hot, index, 0);
    } else if (temp >= 60) {
      int index = (temp - 60) / 2;
      displayGauge(temperate, index, 0);
    } else if (temp >= 28) {
      int index = (temp - 28) / 2;
      displayGauge(frigid, index, 0);
    } else if (temp >= -4) {
      int index = temp + 4 / 2;
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
void displayGauge(uint32_t colors[16], int stopIndex, int displayType) {
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
  strip.clear();
  for (int j = 0; j < leds; j++) {
    strip.setPixelColor(j, color);
  }

  int Pixel = random(leds);
  strip.setPixelColor(Pixel, 0xff, 0xff, 0xff, 0xff);
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
