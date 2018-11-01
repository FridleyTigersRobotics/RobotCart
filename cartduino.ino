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
int const sidHorn   = 1;
int const sidLights = 3;
int const sidInvert = 4;
int const sidRadio  = 6;
int const sidXx     = 10;
int const sidPm     = 11;
int const sidAm     = 12;
int const sidVdc    = 14;

// Widget IDs (wid)
int const widHornButton     = 1;
int const widLedSlider      = 2;
int const widLightsCheckbox = 3;
int const widInvertCheckbox = 4;
int const widSliderValue    = 5;
int const widRadioCheckbox  = 6;
int const widRtcSecond      = 7;
int const widRtcMinute      = 8;
int const widRtcHour        = 9;
int const widAmPmText       = 10;
int const widVoltageDisplay = 13;
int const widVdcText        = 14;

// Theme IDs (tid)
int const tidTheme0 = 1;
int const tidTheme1 = 2;
int const tidTheme2 = 3;
int const tidTheme3 = 5;
int const tidTheme4 = 10;

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
  lcd.theme( tidTheme3, 1, 2, 0, 3, 3,  4, 4,  5,  6, 0 );// what font??
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




#if MANUAL_LED_DRIVER


class LedAnimator
{

private:
  int length;
  int numStrips;
  int numLeds;
  CRGB *leds;


public:

  typedef enum
  {
     LED_ANI_Solid,
     NUM_LED_ANIMATIONS
  } led_animation_t;


  LedAnimator( 
    int ledStripLength,
    int numLedStrips
  )
  { 
    length    = ledStripLength;
    numStrips = numLedStrips;
    numLeds   = ledStripLength * numLedStrips;
    leds      = new CRGB[ numLeds ];
  }

  ~LedAnimator()
  {
    delete []leds;
  }



  void Animation_Solid(

  )
  {



  }




};










int const ledStripLength = 60;
int const numLedStrips   = 4;
int const totalNumLeds   = ledStripLength * numLedStrips;

//float const ledSignalPropgationTime = totalNumLeds;

int const ledDriverPin = 6;








//***************************************************************************
//  
//
//
//***************************************************************************
void initializeLedDriver()
{
  //FastLED.addLeds< NEOPIXEL, ledDriverPin >( leds, totalNumLeds );
}




//***************************************************************************
//  
//
//
//***************************************************************************
void updateLedDriver()
{







  //FastLED.show()
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

  updateRelays();

  updateRealTimeClock( now );

  updateBatteryVoltage( now );

  // slow execution delay before next loop
  delay( 10 );
}
