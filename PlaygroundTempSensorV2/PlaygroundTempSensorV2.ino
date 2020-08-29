#include <Adafruit_CircuitPlayground.h>
/*
 * This will light each light for each 10 degrees that it detects. It can only go up to
 * 100 degrees because there are only 10 neopixels on the CircuitPlayground Classic.
 * 
 * If temp goes below zero or above 99 then the lights will start flashing, letting you
 * know you are in a dangerous temp range.
 * 
 * The lights will also be colored based on the temperature range. 
 */

const int brightnessLow = 20;
const int brightnessHigh = 255;
const int blinkTimer = 250; // units of milliseconds

int  tempAdjust = -8;  // calibrated based on my household thermostat
int delayCount = 0;
bool lastLeftBtn = false;
bool lastRightBtn = false;

enum BlinkState {Disabled=0, TurnOn=1, TurnOff=2, GoBackToTurnOn=3};

void setup() {
  CircuitPlayground.begin();
  CircuitPlayground.setBrightness(brightnessLow);  
  CircuitPlayground.clearPixels();
  CircuitPlayground.redLED(false);
 
}

BlinkState getBlinkState(bool startBlinking, bool stopBlinking)
{
  static BlinkState blinkState=Disabled;
  static unsigned long blinkLastTime=0;

  if ( stopBlinking )
  {
    blinkState=Disabled;
    blinkLastTime=0;
    return blinkState; // Disable Blinking
  }
  
  if ( startBlinking && blinkState==Disabled )
  {
    blinkState=TurnOn;
    blinkLastTime=0;
  }

  if ( blinkState && ((millis() - blinkLastTime) > blinkTimer) )
  {    
    switch (blinkState)
    {
      case TurnOn:
        blinkState = TurnOff;
        CircuitPlayground.clearPixels();
        break;

       case TurnOff:
        blinkState = TurnOn;
        break;
    }

    blinkLastTime = millis();
  }
  return blinkState;
}

bool areWeInBlinkTempRange(float temp)
{
  return ( temp < 0 || temp > 99);
}

void chooseColorBasedOnTemp( float temp, uint16_t *redO, uint16_t *greenO, uint16_t *blueO, uint16_t *uintTempO )
{
  uint16_t red,green,blue;    // RGB value that indicates temperature range
  uint16_t uintTemp = 0;

  //
  // Each temp range has it's own color
  // < 32 is white (freezing,blinking)
  // < 50 is skyblue (colder)
  // < 60 is solid blue (cold)
  // < 70 is yellow (chilly)
  // < 80 is green (this is a good temp)
  // < 90 is red (warm)
  // > 90 is blinking red - all lights blink - and alarm goes off
  //
  if ( temp >= 0 )
  {
     uint16_t utemp = (uint16_t) temp;
     uintTemp = utemp/10;
     if (uintTemp > 9) 
     {
        uintTemp = 9;
     }
  }

  if ( temp < 0 )
  {
     red=0; green=0; blue=255;
     uintTemp=0;
  }
  else if ( temp < 32 )
  {
     red = 255;  green = 255;  blue = 255;
  } 
  else if ( temp < 50 ) 
  {
     red = 0;   green = 255;  blue = 255;
  } 
  else if ( temp < 60 )
  {
     red = 0;  green = 0;  blue = 255;
  }
  else if ( temp < 70 )
  {
     red = 255;  green = 255;  blue = 0;
  }
  else if ( temp < 80 )
  {
     red = 0;  green = 255;  blue = 0;
  }
  else if ( temp < 90.0 )
  {
     red = 100;  green = 0; blue = 0;
  }
  else
  {
    uintTemp = 9; // Turn on all the neoPixels
    red = 255;  green = 0; blue = 0;
  }

  *redO = red;
  *greenO = green;
  *blueO = blue;
  *uintTempO = uintTemp;

  return;
}

void loop() {
 
  float temp =  CircuitPlayground.temperatureF()  ;		// grab the temperature value from the temp sensor
  uint8_t leftBtn = CircuitPlayground.leftButton();		// adjust temp value by -1 degree
  uint8_t rightBtn = CircuitPlayground.rightButton();	// adjust temp value by +1 degree
  static bool lastNeoDim = true;
  bool neoDim = CircuitPlayground.slideSwitch();             // LEFT=true, RIGHT=false  To change Brightness of NeoPixels; LEFT=dim RIGHT=bright
//  uint16_t soundValue = CircuitPlayground.soundSensor();
//  uint16_t lightValue = CircuitPlayground.lightSensor(); // between 0 and 1023 
  uint16_t red,green,blue;		// RGB value that indicates temperature range
  static BlinkState blinkState=Disabled; // 0=Disabled, 1=LightsOn,  2=LightsOff
  uint16_t uintTemp = 0;
  static uint16_t lastuintTemp=1000; // to make sure our current temperature doesn't match our last temperature
  bool tempChanged=false;
  bool showLights=false;

  if ( neoDim != lastNeoDim )
  {
    CircuitPlayground.playTone(500, 500);
    lastNeoDim = neoDim;
    if ( neoDim ) {
      CircuitPlayground.setBrightness(brightnessLow);
    } else {
      CircuitPlayground.setBrightness(brightnessHigh);
    }
    lastuintTemp = 1000;
    delayCount = 0;
  }
  /*
  ** Adjust the sensor value up or down to help calibrate it
  */
  
  if ( leftBtn && !lastLeftBtn ) {
    tempAdjust--;
    delayCount=0;
    CircuitPlayground.playTone(700, 300);
  } else if ( rightBtn && !lastRightBtn) {
    tempAdjust++;
    delayCount=0;
    CircuitPlayground.playTone(900, 300);
  }
  temp += tempAdjust;

  //
  // You can calibrate the temp reading by using either the LEFT button to decrease the value
  // or the RIGHT button to increase the value. Just look at an accurate temp gauge and 
  // adjust yours to match it
  //
  lastLeftBtn = leftBtn;
  lastRightBtn = rightBtn;

  if ( areWeInBlinkTempRange(temp) )
  {
    blinkState = getBlinkState(1,0);
  }
  else
  {
    (void) getBlinkState(0,1);
    blinkState = Disabled;
  }

  if ( lastuintTemp != uintTemp )
  {
    tempChanged = true;
    lastuintTemp = uintTemp;
  }
  
  if (TurnOn==blinkState)
  {
    showLights = true;
  }
  else if (TurnOff==blinkState)
  {
    showLights = false;
    CircuitPlayground.clearPixels();
  }

  if (Disabled == blinkState)
  {
    if ( delayCount <= 3000 )
    {
      if ( tempChanged )
      {
        showLights = true;
      }
    }
    else if ( delayCount > 3000 )
    {
      showLights = false;
    }
    
    if ( delayCount == 0 )
    {
      lastuintTemp=1000; // to make sure our current temperature doesn't match our last temperature
      tempChanged = true;
    }
  }
  
  if ( tempChanged )
  {
    // need to clear pixels in case the number of pixels to light up has changed
    CircuitPlayground.clearPixels();    
  }
  
  if ( showLights ) 
  {
    chooseColorBasedOnTemp(temp, &red, &green, &blue, &uintTemp);
    for ( uint16_t n = 0; n <= uintTemp ; n++)
    {
      CircuitPlayground.setPixelColor( n, red, green, blue);
    }
  }

  if (Disabled == blinkState) 
  {
    if ( delayCount == 3000 )
    {
       CircuitPlayground.clearPixels();
    }
    else if ( delayCount > 10000 )
    {
      delayCount = -100; // start showing the lights again - to show the temperature
    }
    
    delayCount += 100; // 100 milli second delay
  }
  
  delay(100);
 
}
