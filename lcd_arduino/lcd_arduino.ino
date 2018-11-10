#include <ezLCDLib.h>
#include <DS3231.h>
#include <Wire.h>

#define LED_PIN 13
#define METERDIGITS 2
#define METERDP 0

DS3231 Clock; //Clock object to set time
ezLCD3 lcd; //LCD object
RTClib RTC; //RTC object
int ledPatternSel   = 0;
int sliderval    = 125;
int lastloophour = 99;
int adjusthour   = 99;

int   lastloopsecond     = 99;
float battVoltageSampSum = 0.0;
int   battVoltageSampCnt = 0;
char  numLEDPatterns=9;


// String IDs (sid)
enum string_ids
{
  sidHorn = 1,
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
  widHornButton = 1,
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
  widVdcText,
  widTouchZone
};

// Theme IDs (tid)
enum theme_ids
{
  tidTheme0 = 1,
  tidTheme1,
  tidTheme2,
  tidTheme3,
  tidTheme4
};

// Font IDs (fid)
int const fidStaticFont  = 1;
int const fidDynamicFont = 2;
int const fidMechFont    = 2;

int const DRAW_UNCHECKED = 1;



void initializeLcdDisplay()
{
  lcd.begin( EZM_BAUD_RATE );
  lcd.cls( BLACK, WHITE );
  // Is this a user font, or the build in font 0?

  lcd.picture( 136, 37, "tiger3.gif" );

  lcd.font( "mech" );
  lcd.printString( "Fridley Tigers" );
  lcd.font( "mech2" );
  // Position for next string:
  lcd.print( "\\[30y" );
  lcd.print( "\\[0x" );
  lcd.printString( "2 2 2 7" );


  lcd.fontw( fidMechFont, "mech" );
  lcd.fontw( fidStaticFont, "LCD24" );
  lcd.fontw( fidDynamicFont, 0 );


  lcd.theme( tidTheme0, 1, 3, BLACK, BLACK, BLACK, ORANGE, YELLOW, 1, 1, fidStaticFont );
  lcd.theme( tidTheme3, 1, 2, 0, 3, 3,  4, 4,  5,  6, fidDynamicFont );
  lcd.theme( tidTheme2, 1, 2, 0, 3, 0,  6, 4, 23, 35, fidStaticFont );
  lcd.theme( tidTheme1, 2, 3, 3, 3, 3, 35, 6,  2,  0, fidDynamicFont );
  lcd.theme( tidTheme4, 0, 0, 0, 1, 2,  1, 0,  0,  6, fidStaticFont );

  lcd.string( sidHorn,   "Horn"   );
  lcd.string( sidLights, "Lights" );
  lcd.string( sidInvert, "Invert" );
  lcd.string( sidRadio,  "Radio"  );
  lcd.string( sidXx,     "XX"     );
  lcd.string( sidPm,     "PM"     ); 
  lcd.string( sidAm,     "AM"     );
  lcd.string( sidVdc,    "VDC"    );

  lcd.button( widHornButton, 210, 55, 100, 80, 1, 0, 20, tidTheme0, sidHorn );


  //draw digital meter
  lcd.digitalMeter( widSliderValue, 150, 100, 50, 30, 1, 0, 4, METERDP, tidTheme3 );


  //lcd.checkbox( ID, x, y, width, height, option, theme, string);
  lcd.checkBox( widLightsCheckbox, 1, 60,  130, 50, DRAW_UNCHECKED, tidTheme2, sidLights );
  lcd.checkBox( widInvertCheckbox, 1, 120, 130, 50, DRAW_UNCHECKED, tidTheme2, sidInvert );
  lcd.checkBox( widRadioCheckbox,  1, 180, 130, 50, DRAW_UNCHECKED, tidTheme2, sidRadio  );

  lcd.slider( widLedSlider, 150, 145, 160, 40, 5, 125, 1, 0, tidTheme1 );

  lcd.digitalMeter( widRtcSecond, 230, 200, 30, 30, 14, 0, METERDIGITS, METERDP, tidTheme3 );
  lcd.digitalMeter( widRtcMinute, 190, 200, 30, 30, 14, 0, METERDIGITS, METERDP, tidTheme3 );
  lcd.digitalMeter( widRtcHour,   150, 200, 30, 30, 14, 0, METERDIGITS, METERDP, tidTheme3 );
  lcd.staticText( widAmPmText, 270, 200, 30, 30, 4, tidTheme4, sidXx );


  lcd.digitalMeter( widVoltageDisplay, 200, 10, 60, 30, 14, 0, 4, 2, tidTheme3 );

  lcd.staticText( widVdcText, 270, 10, 45, 30, 4, tidTheme4, sidVdc );

  lcd.touchZone( widTouchZone,  150, 40, 50, 90, 1 );

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
    }
    else 
    {
      digitalWrite( 5, HIGH );
    }    
  }

  if( widgetId == widLightsCheckbox ) 
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

  if( widgetId == widInvertCheckbox ) 
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

  if( widgetId == widRadioCheckbox ) 
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

  if( widgetId == widTouchZone ) 
  {
    if( lcd.currentInfo == PRESSED )
    {
      ledPatternSel ++;
      ledPatternSel %= numLEDPatterns;
    }
  }

}


//***************************************************************************
//  
//
//
//***************************************************************************
void updateLEDdriver()
{
  //set pwm out pin 9
  int const sliderDsiplayValue = sliderval + (ledPatternSel * 1000);
  sliderval = 125 + lcd.wvalue( widLedSlider );
  lcd.wvalue( widSliderValue, sliderDsiplayValue );//lcd.wvalue( widSliderValue, sliderval );
  analogWrite( 9, sliderval );
}

//***************************************************************************
//  
//
//
//***************************************************************************
void setup()
{
  initializeLcdDisplay();

  pinMode( 4, OUTPUT );
  pinMode( 5, OUTPUT );
  pinMode( 6, OUTPUT );
  pinMode( 7, OUTPUT );
  pinMode( LED_PIN, OUTPUT );
  pinMode( 9, OUTPUT );//490Hz PWM (2040.8us period)

  digitalWrite( 4, HIGH );
  digitalWrite( 5, HIGH );
  digitalWrite( 6, HIGH );
  digitalWrite( 7, HIGH );
  digitalWrite( LED_PIN, LOW );
  analogWrite( 9, 125 );//analogWrite values from 0 to 255

  // RTC begin communication
  Wire.begin();

  if( 0 )
  { // set time
    Clock.setYear( 2018 );
    Clock.setMonth( 11 );
    Clock.setDate( 9 );
    Clock.setDoW( 6 );
    Clock.setHour( 6 + 12 );
    Clock.setMinute( 10 );
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

  updateRelays();
  
  updateRealTimeClock( now );
  
  updateBatteryVoltage( now );
  
  updateLEDdriver();
  
}
