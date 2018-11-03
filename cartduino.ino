//Andrew Panning 9/29/2018

// Just a test of editing this file from the GitHub website.


#define MANUAL_LED_DRIVER  ( 1 )
#define BLINKIN_LED_DRIVER ( !MANUAL_LED_DRIVER )

#include <ezLCDLib.h>
#include <DS3231.h>
#include <Wire.h>

#if MANUAL_LED_DRIVER
  #include <FastLED.h>
#endif

#define LED_PIN 13
#define METERDIGITS 2
#define METERDP 0
#define LEDX 175
#define LEDY 70

DS3231 Clock; //Clock object to set time
ezLCD3 lcd; //LCD object
RTClib RTC; //RTC object
int sliderval = 125;
unsigned long previousMillis = 0;
bool updatehour = 0;
int lastloophour = 99;
int adjusthour = 99;

int lastloopsecond = 99;
float battVoltageSampSum = 0.0;
int   battVoltageSampCnt = 0;


// String IDs (sid)
enum string_ids
{
  sidHorn,
  sidLights,
  sidInvert,
  sidRadio,
  sidXx,
  sidPm,
  sidAm,
  sidVdc
};

// Widget IDs (wid)
enum widget_ids
{
  widHornButton,
  widLedSlider,
  widLightsCheckbox,
  widInvertCheckbox,
  widSliderValue,
  widRadioCheckbox,
  widRtcSecond,
  widRtcMinute,
  widRtcHour,
  widAmPmText,
  widVoltageDisplay,
  widVdcText
};

// Theme IDs (tid)
enum theme_ids
{
  tidTheme0,
  tidTheme1,
  tidTheme2,
  tidTheme3,
  tidTheme4
};

// Font IDs (fid)
int const fidFont0 = 1;


int const DRAW_UNCHECKED = 1;



void initializeLcdDisplay()
{
  lcd.begin( EZM_BAUD_RATE );
  lcd.cls( BLACK, WHITE );
  // Is this a user font, or the build in font 0?
  lcd.font( "0" );
  lcd.printString( "Fridley Tiger Robotics" );

  // Position for next string:
  lcd.print( "\\[20y" );
  lcd.print( "\\[0x" );
  lcd.printString( "#2227" );

  lcd.fontw( fidFont0, "LCD24" );


  lcd.theme( tidTheme0, 1, 3, BLACK, BLACK, BLACK, ORANGE, YELLOW, 1, 1, fidFont0 );
  lcd.theme( tidTheme3, 1, 2, 0, 3, 3,  4, 4,  5,  6, fidFont0 );// what font??
  lcd.theme( tidTheme2, 1, 2, 0, 3, 0,  6, 4, 23, 35, fidFont0 );// theme 3
  lcd.theme( tidTheme1, 2, 3, 3, 3, 3, 35, 6,  2,  0, fidFont0 );//theme 2
  lcd.theme( tidTheme4, 0, 0, 0, 1, 2,  1, 0,  0,  6, fidFont0 );

  lcd.string( sidHorn,   "HORN"   );
  lcd.string( sidLights, "Lights" );
  lcd.string( sidInvert, "Invert" );
  lcd.string( sidRadio,  "Radio"  );
  lcd.string( sidXx,     "XX"     );
  lcd.string( sidPm,     "PM"     ); 
  lcd.string( sidAm,     "AM"     );
  lcd.string( sidVdc,    "VDC"    );

  lcd.button( widHornButton, 210, 55, 100, 80, 1, 0, 20, tidTheme0, sidHorn );

  lcd.drawLed( LEDX, LEDY, 12, BLACK, WHITE );


  //draw digital meter 5
  lcd.digitalMeter( widSliderValue, 150, 100, 50, 30, 14, 0, 3, METERDP, tidTheme3 );


  //lcd.checkbox( ID, x, y, width, height, option, theme, string);
  lcd.checkBox( widLightsCheckbox, 1, 60,  130, 50, DRAW_UNCHECKED, tidTheme2, sidLights );
  lcd.checkBox( widInvertCheckbox, 1, 120, 130, 50, DRAW_UNCHECKED, tidTheme2, sidInvert );
  lcd.checkBox( widRadioCheckbox,  1, 180, 130, 50, DRAW_UNCHECKED, tidTheme2, sidRadio  );

  lcd.slider( widLedSlider, 150, 145, 160, 40, 5, 125, 1, 0, tidTheme1 );

  lcd.digitalMeter( widRtcSecond, 230, 200, 30, 30, 14, 0, METERDIGITS, METERDP, tidTheme3 );
  lcd.digitalMeter( widRtcMinute, 190, 200, 30, 30, 14, 0, METERDIGITS, METERDP, tidTheme3 );
  lcd.digitalMeter( widRtcHour,   150, 200, 30, 30, 14, 0, METERDIGITS, METERDP, tidTheme3 );
  lcd.staticText( widAmPmText, 270, 200, 30, 30, 4, 10, sidXx );


  lcd.digitalMeter( widVoltageDisplay, 200, 10, 60, 30, 14, 0, 4, 2, tidTheme3 );

  lcd.staticText( widVdcText, 270, 10, 45, 30, 4, 10, sidVdc );

}



//***************************************************************************
//  
//
//
//***************************************************************************
void updateBatteryVoltage(
  DateTime now
)
{
  // scales x11 due to voltage divider, measured x11.243
  float const voltageDividerRatio      = 11.243;
  float const voltageDisplayMultiplier = 100;
  // arduino reads 0.181V high
  float const analogVoltageBias        = 0.181;

  battVoltageSampSum += ( ( (float) analogRead( A0 ) * 5.0f / 1024.0f ) - analogVoltageBias );
  battVoltageSampCnt++;

  // read battery voltage with 1M & 100k resistor 11 voltage divider
  if( lastloopsecond != now.second() )
  {
    float const rawVoltage     = battVoltageSampSum / (float)battVoltageSampCnt;
    float const actualVoltage  = rawVoltage * voltageDividerRatio;
    int   const displayVoltage = actualVoltage * voltageDisplayMultiplier;

    lcd.wvalue( widVoltageDisplay, displayVoltage );

    battVoltageSampCnt = 0;
    battVoltageSampSum = 0.0f;
    lastloopsecond     = now.second();
  }
}



//***************************************************************************
//  
//
//
//***************************************************************************
void updateRealTimeClock( 
  DateTime now 
)
{
  if( lastloophour != now.hour() )
  {
    lastloophour = now.hour();
    if( now.hour() > 11 )
    {
      lcd.st_value( widAmPmText, sidPm );
    }
    else
    {
      lcd.st_value( widAmPmText, sidAm );
    }

    if( now.hour() > 12 )
    {
      adjusthour = now.hour() - 12;
    }
    else if ( now.hour() == 0 )
    {
      adjusthour = 12;
    }
    else if( now.hour() <= 12 && now.hour() != 0 )
    {
      adjusthour = now.hour();
    }
  }
 
  lcd.wvalue( widRtcSecond, now.second() );
  lcd.wvalue( widRtcMinute, now.minute() );
  lcd.wvalue( widRtcHour,   adjusthour   );  
}



//***************************************************************************
//  
//
//
//***************************************************************************
void updateRelays()
{
  int const widgetId = lcd.wstack( FIFO );

  if( widgetId == widHornButton ) 
  {
    if( lcd.currentInfo == PRESSED )
    {
      digitalWrite( 5, LOW );
      lcd.drawLed( LEDX, LEDY, 12, RED, WHITE);
    }
    else 
    {
      digitalWrite( 5, HIGH );
      lcd.drawLed( LEDX, LEDY, 12, BLACK, WHITE);
    }    
  }

  if( widgetId == widLightsCheckbox ) 
  { 
    if( lcd.currentInfo == PRESSED )
    {
      digitalWrite( 4, LOW );
    }
    else
    {
      digitalWrite( 4, HIGH );      
    }
  }

  if( widgetId == widInvertCheckbox ) 
  {
    if( lcd.currentInfo == PRESSED )
    {
      digitalWrite( 6, LOW );
    }
    else
    {
      digitalWrite( 6, HIGH );      
    }
  }

  if( widgetId == widRadioCheckbox ) 
  {
    if( lcd.currentInfo == PRESSED )
    {
      digitalWrite( 7, LOW );
    }
    else
    {
      digitalWrite( 7, HIGH );      
    }
  }
}

  typedef enum
  {
     LED_ANI_None,
     LED_ANI_Solid,
     LED_ANI_Raindow,
     LED_ANI_Pong,
     LED_ANI_Vu,
     LED_ANI_Rain,
     NUM_LED_ANIMATIONS
  } led_animation_t;



#if MANUAL_LED_DRIVER

float const brightArray[16] = {0.0, 0.1, 0.2, 0.3, 0.5, 0.7, 0.8, 0.9, 0.9, 0.8, 0.7, 0.5, 0.3, 0.2, 0.1, 0.0};

class LedAnimator
{

private:
  int length;
  int numStrips;
  int numLeds;
  CRGB *leds;
  float timeIncrement;
  float currentTime;
  led_animation_t currentAnimation;


public:



  LedAnimator( 
    int ledStripLength,
    int numLedStrips
  )
  { 
    length        = ledStripLength;
    numStrips     = numLedStrips;
    numLeds       = ledStripLength * numLedStrips;
    leds          = new CRGB[ numLeds ];     
    timeIncrement = 1.0f;
    currentTime   = 0.0f;
    currentAnimation = LED_ANI_Vu;
    FastLED.addLeds< NEOPIXEL, 9 >( leds, numLeds );
  }

  ~LedAnimator()
  {
    delete []leds;
  }

  void Init_Animation()
  {
    switch ( currentAnimation )
    {
      case LED_ANI_Solid:
      {
        Init_Solid();
        break;
      }

      case LED_ANI_Raindow:
      {
        Init_Rainbow();
        break;
      }

      case LED_ANI_Pong:
      {
        Init_Pong( 0 );
        break;
      }

      case LED_ANI_Vu:
      {
        Init_Vu( );
        break;
      }

      case LED_ANI_Rain:
      {
        Init_Rain( );
        break;
      }


      case LED_ANI_None:
      default:
      {
        TurnOffLeds();
        break;
      }
    }
  }

  void Update_Animation()
  {
    currentTime += timeIncrement;

    switch ( currentAnimation )
    {
      case LED_ANI_Solid:
      {
        Animation_Solid();
        break;
      }

      case LED_ANI_Raindow:
      {
        Animation_Rainbow( currentTime );
        break;
      }

      case LED_ANI_Pong:
      {
        Animation_Pong( currentTime );
        break;
      }

      case LED_ANI_Vu:
      {
        Animation_Vu( currentTime );
        break;
      }

      
      case LED_ANI_Rain:
      {
        Animation_Rain( currentTime );
        delay( 50 );
        break;
      }

      case LED_ANI_None:
      default:
      {
        break;
      }
    }
    
    FastLED.show();
  }


  void TurnOffLeds()
  {
    for ( int idx = 0; idx < numLeds; idx++ )
    {
      leds[idx] = CRGB::Black;
    }
  }


  void Init_Solid( )
  {
    for ( int idx = 0; idx < numLeds; idx++ )
    {
      leds[idx] = CRGB::Black;
    }
  }

  void Animation_Solid()
  {
    for ( int idx = 0; idx < numLeds; idx++ )
    {
      leds[idx] = CRGB::Black;
    }
  }

  void Init_Rainbow( )
  {

  }

  void Animation_Rainbow( float currentTime )
  {
    uint8_t const offset = floor( currentTime );

    for ( int idx = 0; idx < numLeds; idx++ )
    {
      uint8_t const hue = offset + idx*2;
      leds[idx].setHSV( hue, 255, 90 );
    }
  }

 
  int *rain;

  void Init_Rain( )
  {
    rain = new int[length];
    //memset( rain, 0, sizeof(*rain) * length );
    srand(millis());
  }

  void Animation_Rain( float currentTime )
  {
    int const dist = 4;

    for ( int idx = 0; idx < length; idx++ )
    {
      if ( rain[idx] == 0 )
      {
        if ((rand()%1000) == 0)
        {
          rain[idx] = 1;
        }
      }
      else if ( rain[idx] > (14 + dist) )
      {
        rain[idx] = 0;
      }
      else
      {
        rain[idx]++;
      }
    }
    float bightness = 190;
    uint8_t hue = 96;
    for ( int idx = 0; idx < length; idx++ )
    {
      int rainValue = rain[idx];
      if ( rain[idx] == 0 )
      {
        leds[idx].setHSV( hue, 255, 0 );
        leds[length*2 - idx - 1].setHSV( hue, 255, 0 );
      }
      else
      {
        int topIdx = (rainValue < 16) ? rainValue : 15;
        int botIdx = ((rainValue-dist) > 0) ? (rainValue-dist) : 0;
        uint8_t topBri = bightness * brightArray[topIdx];
        uint8_t botBri = bightness * brightArray[botIdx];
        leds[idx].setHSV( hue, 255, topBri );
        leds[length*2 - idx - 1].setHSV( hue, 255, botBri );
      }
    }

  }




  
  float animationLen;
  int   vuLedLen;
  float vuTime;
  void Init_Vu( )
  {
    animationLen = 35;
    vuLedLen = 60;
    vuTime = 0;
    TurnOffLeds();
  }

  void Animation_Vu( float currentTime )
  {
    uint8_t const numStrips2 = (numLeds + vuLedLen) / vuLedLen - 1;

    vuTime += 1.0f;
    if ( vuTime >= animationLen )
    {
       vuTime = 0;
    }

    

    for ( int idx = 0; idx < vuLedLen; idx++ )
    {
      int const minPos = 0;
      int maxPos;
      uint8_t bightness = 100;
      uint8_t hue = 96;

      if (vuTime < (animationLen/2.0))
      {
        maxPos = ceil( (float)vuLedLen * (-0.25 * vuTime + 0.05 *vuTime * vuTime) / (animationLen/2.0) );
      }
      else
      {
        float vuTime2 = (animationLen) - vuTime;
        maxPos = ceil( (float)vuLedLen * (vuTime2 / (animationLen/2.0) ));
      }

    
      if ( idx > 40 )
      {
        hue = 45;
      }
      
      if ( idx > 50 )
      {
        hue = 0;
      }

      if ( ( idx > minPos ) && ( idx < maxPos ) )
      {
        bightness = 80;
      }
      else
      {
        bightness = 0;
      }
      
      for ( int idxStrip = 0; idxStrip < numStrips2; idxStrip++ )
      {
        if (idxStrip % 2)
        {
          leds[idx + vuLedLen * idxStrip].setHSV( hue, 255, bightness );
        }
        else
        {
           leds[vuLedLen * (idxStrip+1)-idx - 1].setHSV( hue, 255, bightness );
        }
      }
    }
  }

  


  uint8_t  pongHue;
  uint16_t pongPosition;
  int      pongWidth;
  int      pongInc;
  int      pongBoundry;

  void Init_Pong( uint8_t hue )
  {
    pongHue      = hue;
    pongPosition = 0;
    pongWidth    = 4;
    pongInc      = 1;
    pongBoundry  = 30;
    TurnOffLeds();
  }

  void Animation_Pong( float currentTime )
  {
    uint8_t const offset = floor( currentTime );
    uint8_t const numStrips2 = (numLeds + pongBoundry) / pongBoundry - 1;
    for ( int idx = 0; idx < pongBoundry; idx++ )
    {
      int const minPos = pongPosition - pongWidth;
      int const maxPos = pongPosition + pongWidth;
      uint8_t bightness = 0;

      if ( ( idx > minPos ) && ( idx < maxPos ) )
      {
        bightness = 90;
      }
      else
      {
        bightness = 0;
      }
      
      for ( int idxStrip = 0; idxStrip < numStrips2; idxStrip++ )
      {
        if (idxStrip % 2)
        {
          leds[idx + pongBoundry * idxStrip].setHSV( pongHue, 255, bightness );
        }
        else
        {
           leds[pongBoundry * (idxStrip+1)-idx-1].setHSV( pongHue, 255, bightness );
        }
      }
    }

    pongPosition += pongInc;
    if ( pongPosition >= pongBoundry )
    {
      pongInc *= -1;
    }
  }




};










int const ledStripLength = 60;
int const numLedStrips   = 4;
int const totalNumLeds   = ledStripLength * numLedStrips;
CRGB leds[totalNumLeds];

//float const ledSignalPropgationTime = totalNumLeds;

int const ledDriverPin = 9;
  

LedAnimator ledDriver = LedAnimator::LedAnimator(ledStripLength, numLedStrips);





//***************************************************************************
//  
//
//
//***************************************************************************
void initializeLedDriver()
{
  //leds          = new CRGB[ totalNumLeds ];
  //FastLED.addLeds< NEOPIXEL, ledDriverPin >( leds, totalNumLeds );

  ledDriver.Init_Animation();

  /*for ( int idx = 0; idx < totalNumLeds; idx++ )
  {
    leds[idx].setHSV( idx % 256, 255, 0 );
  }*/
}



uint8_t  offset = 0;
int count2 = 0;
//***************************************************************************
//  
//
//
//***************************************************************************
void updateLedDriver()
{
  ledDriver.Update_Animation();

  /*for ( int idx = 0; idx < totalNumLeds; idx++ )
  {
    uint8_t b = 0;
    uint8_t a = 50;
    uint8_t c = 255;

    if ( offset )
    {
      b = 0;
    }
    else
    {
      b = 255;
    }
    
    //leds[idx].setHSV( a, c, b );
  }
  uint8_t hue = 100;
  uint8_t sat = 255;
  uint8_t bri = 100;

  
  leds[offset].setHSV( hue, sat, bri );
  leds[offset+1].setHSV( hue, sat, bri );
  
  FastLED.show();
  leds[offset].setHSV( hue, sat, 0 );
  leds[offset+1].setHSV( hue, sat, 0 );
  offset+=2;
  offset %= totalNumLeds;*/
}

#endif // #if MANUAL_LED_DRIVER


#if BLINKIN_LED_DRIVER
//***************************************************************************
//  
//
//
//***************************************************************************
void updateBlinkin()
{
  unsigned long const pwmUpdateInterval = 100;
  unsigned long const currentMillis     = millis();

  //set pwm out pin 9 for Blinkin
  sliderval = 125 + lcd.wvalue( 2 );
  lcd.wvalue( widSliderValue, sliderval );
  
  if( currentMillis - previousMillis > pwmUpdateInterval )
  {//only update pwm every 100 milliseconds
    previousMillis = currentMillis;
    analogWrite( 9, sliderval );
  }
}

#endif // #if BLINKIN_LED_DRIVER




//***************************************************************************
//  
//
//
//***************************************************************************
void setup()
{
  initializeLcdDisplay();

#if MANUAL_LED_DRIVER
  initializeLedDriver();
#endif

#if BLINKIN_LED_DRIVER
  pinMode( 9, OUTPUT );//490Hz PWM (2040.8us period)
#endif

  pinMode( 4, OUTPUT );
  pinMode( 5, OUTPUT );
  pinMode( 6, OUTPUT );
  pinMode( 7, OUTPUT );
  pinMode( LED_PIN, OUTPUT );

  digitalWrite( 4, HIGH );
  digitalWrite( 5, HIGH );
  digitalWrite( 6, HIGH );
  digitalWrite( 7, HIGH );
  digitalWrite( LED_PIN, LOW );

#if BLINKIN_LED_DRIVER
  analogWrite( 9, 125 );//analogWrite values from 0 to 255,
#endif

  // RTC begin communication
  Wire.begin();

  if( 0 )
  { // set time
    Clock.setYear( 2018 );
    Clock.setMonth( 9 );
    Clock.setDate( 23 );
    Clock.setDoW( 1 );
    Clock.setHour( 8 + 12 );
    Clock.setMinute( 39 );
    Clock.setSecond( 0 );
  }
}


int long updateMilli = 0;

//***************************************************************************
//  
//
//
//***************************************************************************
void loop()
{
  DateTime now = RTC.now();

#if MANUAL_LED_DRIVER
  updateLedDriver();
#endif

#if BLINKIN_LED_DRIVER
  updateBlinkin();
#endif

  if ( (millis() - updateMilli) > 2000 )
  {
    updateMilli = millis();
    //updateRelays();
  
    //updateRealTimeClock( now );
  
    //updateBatteryVoltage( now );
  }

  // slow execution delay before next loop
  //delay( 5 );
}
