#include <SHA256.h>
#include <DS3231.h>

#include <ezLCDLib.h>
#include <Wire.h>

#define LED_PIN 13
#define METERDIGITS 2
#define METERDP 0

DS3231 Clock; //Clock object to set time
ezLCD3 lcd; //LCD object
RTClib RTC; //RTC object

int ledPatternSel   = 0;
int lastledPatternSel = 9;
int sliderval    = 125;
int lastloophour = 99;
int adjusthour   = 99;

int   lastloopsecond     = 99;
float battVoltageSampSum = 0.0;
int   battVoltageSampCnt = 0;
char  numLEDPatterns=10;

unsigned int timeAdjustHour  = 0;
unsigned int timeAdjustMinHi = 0;
unsigned int timeAdjustMinLo = 0;
bool         timeAdjustIsPm  = false;

#define MAX_NUM_NUMPAD_ENTRIES ( 20 )
char numpadEntry[MAX_NUM_NUMPAD_ENTRIES + 1] = { 0 };
int numNumpadEntries = 0;

bool inCompMode = false;

uint8_t correctHash[32] = \
{
  0xeb, 0xa2, 0xf0, 0xfe, 0xac, 0x9c, 0xac, 0xcc, 
  0x83, 0x53, 0x6e, 0xfa, 0x0b, 0x52, 0xe7, 0xc8, 
  0x30, 0xf0, 0xe6, 0x68, 0x1d, 0x66, 0xf1, 0xfb, 
  0x50, 0x1f, 0x08, 0x5e, 0xba, 0x11, 0x33, 0x0f
};

uint8_t hash[32] = { 0 };
SHA256 CompModePass;

typedef enum 
{
  mainScreen,
  optionsScreen,
  numpadScreen,
  timeScreen
} screen_t;

screen_t currentScreen = mainScreen;
screen_t nextScreen    = mainScreen;

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
  sidVdc,
  sidNumpad0,
  sidNumpad1,
  sidNumpad2, 
  sidNumpad3,
  sidNumpad4,
  sidNumpad5,
  sidNumpad6,
  sidNumpad7,
  sidNumpad8,
  sidNumpad9,
  sidNumpadDone,
  sidNumpadExit,
  sidOptionsOptions,
  sidOptionsTime,
  sidOptionsCompMode,
  sidBack,
  sidTimePlus,
  sidTimeMinus,
  sidTimeSet,
  sidNumpadEntry
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
  widOptionsButton,
  widLedPatChgZone,
  widNumpad0,
  widNumpad1,
  widNumpad2, 
  widNumpad3,
  widNumpad4,
  widNumpad5,
  widNumpad6,
  widNumpad7,
  widNumpad8,
  widNumpad9,
  widNumpadDone,
  widNumpadBack,
  widNumpadEntry,
  widOptionsOptions,
  widOptionsTime,
  widOptionsCompMode,
  widOptionsBack,
  widTimeAdjHour,
  widTimeAdjMinuteLo,
  widTimeAdjMinuteHi,
  widTimeAdjAmPm,
  widTimeAdjHourPlus,
  widTimeAdjHourMinus,
  widTimeAdjMinuteLoPlus,
  widTimeAdjMinuteLoMinus,
  widTimeAdjMinuteHiPlus,
  widTimeAdjMinuteHiMinus,
  widTimeAdjAmPmPlus,
  widTimeAdjAmPmMinus,
  widTimeAdjBack,
  widTimeAdjSet
};

// Theme IDs (tid)
enum theme_ids
{
  tidTheme0 = 1,
  tidTheme1,
  tidTheme2,
  tidTheme3,
  tidTheme4,
  tidTheme5
};

// Font IDs (fid)
int const fidStaticFont  = 1;
int const fidDynamicFont = 2;
int const fidMechFont    = 2;

int const DRAW_UNCHECKED = 1;

bool GetCompMode( void )
{
  byte A1Day;
  byte A1Hour;
  byte A1Minute;
  byte A1Second;
  byte AlarmBits;
  bool A1Dy;
  bool A1h12;
  bool A1PM;
  Clock.getA1Time( A1Day, A1Hour, A1Minute, A1Second, AlarmBits, A1Dy, A1h12, A1PM );
  if ( AlarmBits == 0 )
  {
    return false;
  }
  else
  {
    return true;
  }
}

void SetCompMode( bool compMode )
{
  uint8_t value = ( compMode ) ? ( 1 ) : ( 0 );
  Clock.setA1Time( value, 0, 0, 0, value, false, false, false );
}

void DrawMainScreen()
{
  lcd.cls( BLACK, WHITE );

  lastloophour   = 99;
  adjusthour     = 99;
  lastloopsecond = 99;

  lcd.picture( 136, 37, "tiger3.gif" );
  lcd.picture( 290, 205, "gear.gif" );
  lcd.font( "mech" );
  lcd.printString( "Fridley Tigers" );
  lcd.font( "mech2" );
  // Position for next string:
  lcd.print( "\\[30y" );
  lcd.print( "\\[0x" );
  lcd.printString( "2 2 2 7" );

  lcd.button( widHornButton, 210, 55, 100, 80, 1, 0, 20, tidTheme0, sidHorn );
  lcd.digitalMeter( widSliderValue, 150, 100, 50, 30, 1, 0, 4, METERDP, tidTheme3 );

  //lcd.checkbox( ID, x, y, width, height, option, theme, string);
  lcd.checkBox( widLightsCheckbox, 1, 60,  130, 50, DRAW_UNCHECKED, tidTheme2, sidLights );
  lcd.checkBox( widInvertCheckbox, 1, 120, 130, 50, DRAW_UNCHECKED, tidTheme2, sidInvert );

  if ( GetCompMode() )
  {
    lcd.checkBox( widRadioCheckbox,  1, 180, 130, 50, 2, tidTheme2, sidRadio  );
  }
  else
  {
    lcd.checkBox( widRadioCheckbox,  1, 180, 130, 50, DRAW_UNCHECKED, tidTheme2, sidRadio  );
  }


  lcd.slider( widLedSlider, 150, 145, 160, 40, 5, 125, 1, 0, tidTheme1 );

  lcd.digitalMeter( widRtcSecond, 220, 200, 28, 30, 14, 0, METERDIGITS, METERDP, tidTheme3 );
  lcd.digitalMeter( widRtcMinute, 185, 200, 28, 30, 14, 0, METERDIGITS, METERDP, tidTheme3 );
  lcd.digitalMeter( widRtcHour,   150, 200, 28, 30, 14, 0, METERDIGITS, METERDP, tidTheme3 );
  lcd.staticText(   widAmPmText,  255, 200, 28, 30, 4, tidTheme4, sidXx );


  lcd.digitalMeter( widVoltageDisplay, 200, 10, 60, 30, 14, 0, 4, 2, tidTheme3 );

  lcd.staticText( widVdcText, 270, 10, 45, 30, 4, tidTheme4, sidVdc );

  lcd.touchZone( widLedPatChgZone, 150,  40, 50, 90, 1 );
  lcd.touchZone( widOptionsButton, 265, 200, 55, 40, 1 );
} 


void DrawNumpadScreen()
{
  lcd.cls( BLACK, WHITE );

  lcd.staticText( widNumpadEntry, 40, 3, 220, 30, 4, tidTheme4, sidNumpadEntry );
  lcd.st_value( widNumpadEntry, "-" );


  // Arrange like a phone keypad
  //                 wid,      x,   y,  w,  h
  lcd.button( widNumpad1,     75,  35, 50, 40, 1, 0, 20, tidTheme0, sidNumpad1    );
  lcd.button( widNumpad2,    135,  35, 50, 40, 1, 0, 20, tidTheme0, sidNumpad2    );
  lcd.button( widNumpad3,    195,  35, 50, 40, 1, 0, 20, tidTheme0, sidNumpad3    );
  lcd.button( widNumpad4,     75,  85, 50, 40, 1, 0, 20, tidTheme0, sidNumpad4    );
  lcd.button( widNumpad5,    135,  85, 50, 40, 1, 0, 20, tidTheme0, sidNumpad5    );
  lcd.button( widNumpad6,    195,  85, 50, 40, 1, 0, 20, tidTheme0, sidNumpad6    );
  lcd.button( widNumpad7,     75, 135, 50, 40, 1, 0, 20, tidTheme0, sidNumpad7    );
  lcd.button( widNumpad8,    135, 135, 50, 40, 1, 0, 20, tidTheme0, sidNumpad8    );
  lcd.button( widNumpad9,    195, 135, 50, 40, 1, 0, 20, tidTheme0, sidNumpad9    );
  lcd.button( widNumpadBack,   5, 185, 60, 40, 1, 0, 20, tidTheme0, sidBack       );
  lcd.button( widNumpad0,    135, 185, 50, 40, 1, 0, 20, tidTheme0, sidNumpad0    );
  lcd.button( widNumpadDone, 260, 185, 60, 40, 1, 0, 20, tidTheme0, sidNumpadDone );
}


void DrawOptionsScreen()
{
  lcd.cls( BLACK, WHITE );

  lcd.staticText( widOptionsOptions, 60, 5, 200, 30, 4, tidTheme4, sidOptionsOptions );

  lcd.button( widOptionsTime,     60,  45, 200, 55, 1, 0, 20, tidTheme0, sidOptionsTime     );
  lcd.button( widOptionsCompMode, 60, 110, 200, 55, 1, 0, 20, tidTheme0, sidOptionsCompMode );
  lcd.button( widOptionsBack,     60, 175, 200, 55, 1, 0, 20, tidTheme0, sidBack     );
}


void DrawTimeScreen()
{
  lcd.cls( BLACK, WHITE );

  //                 wid,      x,   y,  w,  h
  lcd.button( widTimeAdjHourPlus,        45,   5, 50, 50, 1, 0, 20, tidTheme0, sidTimePlus  );
  lcd.button( widTimeAdjMinuteHiPlus,   115,   5, 50, 50, 1, 0, 20, tidTheme0, sidTimePlus );
  lcd.button( widTimeAdjMinuteLoPlus,   165,   5, 50, 50, 1, 0, 20, tidTheme0, sidTimePlus  );
  lcd.button( widTimeAdjAmPmPlus,       240,   5, 50, 50, 1, 0, 20, tidTheme0, sidTimePlus  );

  lcd.digitalMeter( widTimeAdjHour,      45,  65, 50, 50, 14, timeAdjustHour, METERDIGITS, METERDP, tidTheme3 );
  lcd.digitalMeter( widTimeAdjMinuteHi, 115,  65, 50, 50, 14, timeAdjustMinHi, 1, METERDP,          tidTheme3 );
  lcd.digitalMeter( widTimeAdjMinuteLo, 165,  65, 50, 50, 14, timeAdjustMinLo,  1, METERDP,         tidTheme3 );

  lcd.staticText(   widTimeAdjAmPm,     250,  80, 30, 20, 14, tidTheme3, sidXx );

  lcd.button( widTimeAdjHourMinus,       45, 125, 50, 50, 1, 0, 20, tidTheme0, sidTimeMinus );
  lcd.button( widTimeAdjMinuteHiMinus,  115, 125, 50, 50, 1, 0, 20, tidTheme0, sidTimeMinus );
  lcd.button( widTimeAdjMinuteLoMinus,  165, 125, 50, 50, 1, 0, 20, tidTheme0, sidTimeMinus );
  lcd.button( widTimeAdjAmPmMinus,      240, 125, 50, 50, 1, 0, 20, tidTheme0, sidTimeMinus );

  lcd.button( widTimeAdjBack,             5, 185, 60, 50, 1, 0, 20, tidTheme0, sidBack      );
  lcd.button( widTimeAdjSet,            260, 185, 60, 50, 1, 0, 20, tidTheme0, sidTimeSet   );

  if ( timeAdjustIsPm == true )
  {
    lcd.st_value( widTimeAdjAmPm, sidPm );
  }
  else
  {
    lcd.st_value( widTimeAdjAmPm, sidAm );
  }
}




void initializeLcdDisplay()
{
  lcd.begin( EZM_BAUD_RATE );

  lcd.fontw( fidMechFont, "mech" );
  lcd.fontw( fidStaticFont, "LCD24" );
  lcd.fontw( fidDynamicFont, 0 );

  lcd.theme( tidTheme0, 1, 3, BLACK, BLACK, BLACK, ORANGE, YELLOW, 1, 1, fidStaticFont );
  lcd.theme( tidTheme3, 1, 2, 0, 3, 3,  4, 4,  5,  6, fidDynamicFont );
  lcd.theme( tidTheme2, 1, 2, 0, 3, 0,  6, 4, 23, 35, fidStaticFont );
  lcd.theme( tidTheme5, 1, 2, 1, 1, 0,  6, 4, 23, 35, fidStaticFont );
  lcd.theme( tidTheme1, 2, 3, 3, 3, 3, 35, 6,  2,  0, fidDynamicFont );
  lcd.theme( tidTheme4, 0, 0, 0, 1, 2,  1, 0,  0,  6, fidStaticFont );

  lcd.string( sidHorn,       "Horn"   );
  lcd.string( sidLights,     "Lights" );
  lcd.string( sidInvert,     "Invert" );
  lcd.string( sidRadio,      "Radio"  );
  lcd.string( sidXx,         "XX"     );
  lcd.string( sidPm,         "PM"     ); 
  lcd.string( sidAm,         "AM"     );
  lcd.string( sidVdc,        "VDC"    );
  lcd.string( sidNumpad0,    "0"      );
  lcd.string( sidNumpad1,    "1"      );
  lcd.string( sidNumpad2,    "2"      );
  lcd.string( sidNumpad3,    "3"      );
  lcd.string( sidNumpad4,    "4"      );
  lcd.string( sidNumpad5,    "5"      ); 
  lcd.string( sidNumpad6,    "6"      );
  lcd.string( sidNumpad7,    "7"      );
  lcd.string( sidNumpad8,    "8"      );
  lcd.string( sidNumpad9,    "9"      );
  lcd.string( sidNumpadDone, "Enter"  );
  lcd.string( sidNumpadExit, "Exit"   );
  lcd.string( sidOptionsOptions, "Options" );

  lcd.string( sidOptionsTime,     "Adjust Clock"     );
  lcd.string( sidOptionsCompMode, "Competition Mode" );
  lcd.string( sidBack,            "Back"             ); 
  lcd.string( sidTimePlus,        "+"                );
  lcd.string( sidTimeMinus,       "-"                );
  lcd.string( sidTimeSet,         "Set"              );
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
void updateLEDdriver()
{
  //set pwm out pin 9
  int const sliderDsiplayValue = sliderval + (ledPatternSel * 1000);
  sliderval = 125 + lcd.wvalue( widLedSlider );
  lcd.wvalue( widSliderValue, sliderDsiplayValue );
  if( lastledPatternSel != ledPatternSel )
  {
    lastledPatternSel = ledPatternSel;
    unsigned long millicapture = millis();
    int const sendval = ( ledPatternSel + 1 ) * 10;
    while( ( millis() - millicapture ) < 200 )
    {
      analogWrite( 9, sendval );
    }
  }
  analogWrite( 9, sliderval );
}



void ServiceMainScreen( void )
{
  int const widgetId = lcd.wstack( FIFO );


  DateTime now = RTC.now();

  switch ( widgetId )
  {
    case widHornButton:
    {
      if( lcd.currentInfo == PRESSED )
      {
        digitalWrite( 5, LOW );
      }
      else
      {
        digitalWrite( 5, HIGH );      
      }
      break;
    }

    case widLightsCheckbox:
    {
      if( lcd.currentInfo == PRESSED )
      {
        digitalWrite( 7, LOW );
      }
      else
      {
        digitalWrite( 7, HIGH );      
      }
      break;
    }

    case widInvertCheckbox:
    {
      if( lcd.currentInfo == PRESSED )
      {
        digitalWrite( 4, LOW );
      }
      else
      {
        digitalWrite( 4, HIGH );      
      }
      break;
    }

    case widRadioCheckbox:
    {
      if ( inCompMode == false )
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
      break;
    }

    case widLedPatChgZone:
    {
      if( lcd.currentInfo == RELEASED )
      {
        ledPatternSel++;
        ledPatternSel %= numLEDPatterns;
      }
      break;
    }

    case widOptionsButton:
    {
      if( lcd.currentInfo == RELEASED )
      {
        nextScreen = optionsScreen;
      }
      break;
    }

    default:
    {
      break;
    }
  }

  if ( inCompMode )
  {
    digitalWrite( 6, HIGH );
  }

  updateRealTimeClock( now );
  updateBatteryVoltage( now );
  updateLEDdriver();
  
}



void ServiceOptionsScreen( void )
{
  int const widgetId = lcd.wstack( FIFO );

  if ( lcd.currentInfo == RELEASED )
  {
    switch ( widgetId )
    {
      case widOptionsTime:
      {
        nextScreen = timeScreen;
        break;
      }

      case widOptionsCompMode:
      {
        nextScreen = numpadScreen;
        break;
      }

      case widOptionsBack:
      {
        nextScreen = mainScreen;
        break;
      }

      default:
      {
        break;
      }
    }
  }
}



void ServiceTimeScreen( void )
{
  int const widgetId = lcd.wstack( FIFO );
  bool amPmChanged = false;

  if ( lcd.currentInfo == RELEASED )
  {
    switch ( widgetId )
    {
      case widTimeAdjHourPlus:
      {
        if ( timeAdjustHour == 12 )
        {
          timeAdjustHour = 1;
        }
        else
        {
          timeAdjustHour++;
        }
        break;
      }
      case widTimeAdjHourMinus:
      {
        if ( timeAdjustHour == 1 )
        {
          timeAdjustHour = 12;
        }
        else
        {
          timeAdjustHour--;
        }

        break;
      }

      case widTimeAdjMinuteLoPlus:
      {
        if ( timeAdjustMinLo == 9 )
        {
          timeAdjustMinLo = 0;
        }
        else
        {
          timeAdjustMinLo++;
        }

        break;
      }
      case widTimeAdjMinuteLoMinus:
      {
        if ( timeAdjustMinLo == 0 )
        {
          timeAdjustMinLo = 9;
        }
        else
        {
          timeAdjustMinLo--;
        }
        break;
      }

      case widTimeAdjMinuteHiPlus:
      {
        if ( timeAdjustMinHi == 5 )
        {
          timeAdjustMinHi = 0;
        }
        else
        {
          timeAdjustMinHi++;
        }
        break;
      }
      case widTimeAdjMinuteHiMinus:
      {
        if ( timeAdjustMinHi == 0 )
        {
          timeAdjustMinHi = 5;
        }
        else
        {
          timeAdjustMinHi--;
        }
        break;
      }

      case widTimeAdjAmPmPlus:
      case widTimeAdjAmPmMinus:
      {
        amPmChanged = true;
        if ( timeAdjustIsPm == false )
        {
          timeAdjustIsPm = true;
        }
        else
        {
          timeAdjustIsPm = false;
        }
        break;
      }


      case widTimeAdjSet:
      {
        if ( timeAdjustIsPm )
        {
          timeAdjustHour += 12;
        }

        Clock.setHour( timeAdjustHour );
        Clock.setMinute( timeAdjustMinHi * 10 + timeAdjustMinLo );
        Clock.setSecond( 0 );
        nextScreen = mainScreen;
        break;
      }

      case widTimeAdjBack:
      {
        nextScreen = optionsScreen;
        break;
      }

      default:
      {
        break;
      }
    }
  }

  if ( amPmChanged )
  {
    if ( timeAdjustIsPm == true )
    {
      lcd.st_value( widTimeAdjAmPm, sidPm );
    }
    else
    {
      lcd.st_value( widTimeAdjAmPm, sidAm );
    }
  }

  lcd.wvalue( widTimeAdjMinuteLo, timeAdjustMinLo );
  lcd.wvalue( widTimeAdjMinuteHi, timeAdjustMinHi );
  lcd.wvalue( widTimeAdjHour,     timeAdjustHour  );  
}





void ServiceNumpadScreen( void )
{
  int const widgetId = lcd.wstack( FIFO );
  bool numpadEntryChanged = false;
  int  numpadDigit = 0;
  bool checkEntry = false;

  if ( lcd.currentInfo == RELEASED )
  {
    switch ( widgetId )
    {
      case widNumpad0:
      {
        numpadDigit = 0;
        numpadEntryChanged = true;
        break;
      }
      case widNumpad1:
      {
        numpadDigit = 1;
        numpadEntryChanged = true;
        break;
      }
      case widNumpad2:
      {
        numpadDigit = 2;
        numpadEntryChanged = true;
        break;
      }
      case widNumpad3:
      {
        numpadDigit = 3;
        numpadEntryChanged = true;
        break;
      }
      case widNumpad4:
      {
        numpadDigit = 4;
        numpadEntryChanged = true;
        break;
      }
      case widNumpad5:
      {
        numpadDigit = 5;
        numpadEntryChanged = true;
        break;
      }
      case widNumpad6:
      {
        numpadDigit = 6;
        numpadEntryChanged = true;
        break;
      }
      case widNumpad7:
      {
        numpadDigit = 7;
        numpadEntryChanged = true;
        break;
      }
      case widNumpad8:
      {
        numpadDigit = 8;
        numpadEntryChanged = true;
        break;
      }
      case widNumpad9:
      {
        numpadDigit = 9;
        numpadEntryChanged = true;
        break;
      }

      case widNumpadDone:
      {
        checkEntry = true;
        break;
      }

      case widNumpadBack:
      {
        nextScreen = optionsScreen;
        break;
      }

      default:
      {
        break;
      }
    }
  }

  if ( numpadEntryChanged == true )
  {
    numpadEntry[numNumpadEntries] = 48 + numpadDigit;

    numNumpadEntries++;

    lcd.st_value( widNumpadEntry, numpadEntry );

    if ( numNumpadEntries == MAX_NUM_NUMPAD_ENTRIES )
    {
      checkEntry = true;
    }
  }

  if ( checkEntry == true )
  {
    lcd.st_value( widNumpadEntry, "-" );
    CompModePass.reset();
    for ( int idx = 0; idx < MAX_NUM_NUMPAD_ENTRIES; idx++ )
    {
      if ( numpadEntry[idx] == 0x20 )
      {
        numpadEntry[idx] = 65;
      }
      CompModePass.update( &numpadEntry[idx], 1 );
    }
    memset( numpadEntry, 0x20, MAX_NUM_NUMPAD_ENTRIES );
    numNumpadEntries = 0;
    CompModePass.finalize( hash, sizeof( hash ) );

    if ( memcmp( correctHash, hash, sizeof( hash ) ) == 0 )
    {
      if ( inCompMode == true )
      {
        inCompMode = false;
      }
      else
      {
        inCompMode = true;
      }
      SetCompMode( inCompMode );

      nextScreen = mainScreen;
    }
    else
    {
      lcd.st_value( widNumpadEntry, "Incorrect Password" );
    }
  }
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

  inCompMode = GetCompMode();

  DrawMainScreen();
}



//***************************************************************************
//  
//
//
//***************************************************************************
void loop()
{
  if ( currentScreen != nextScreen )
  {
    // Make sure horn is off...
    digitalWrite( 5, HIGH );

    switch ( nextScreen )
    {
      default:
      case mainScreen:
      {
        DrawMainScreen();
        break;
      }

      case optionsScreen:
      {
        DrawOptionsScreen();
        break;
      }

      case numpadScreen:
      {
        memset( numpadEntry, 0x20, MAX_NUM_NUMPAD_ENTRIES );
        numNumpadEntries = 0;
        DrawNumpadScreen();
        break;
      }

      case timeScreen:
      {
        DateTime now = RTC.now();
        timeAdjustHour  = now.hour();
        if ( timeAdjustHour >= 12 )
        {
          timeAdjustIsPm = true;
          timeAdjustHour -= 12;
        }
        timeAdjustMinHi = now.minute() / 10;
        timeAdjustMinLo = now.minute() % 10;

        DrawTimeScreen();
        break;
      }
    }
    currentScreen = nextScreen;
  }



  switch ( currentScreen )
  {
    default:
    case mainScreen:
    {
      ServiceMainScreen();
      break;
    }

    case optionsScreen:
    {
      ServiceOptionsScreen();
      break;
    }

    case numpadScreen:
    {
      ServiceNumpadScreen();
      break;
    }

    case timeScreen:
    {
      ServiceTimeScreen();
      break;
    }
  }
}
