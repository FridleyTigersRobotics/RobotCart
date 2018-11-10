#include <FastLED.h>
#include <bitswap.h>
#include <chipsets.h>
#include <color.h>
#include <colorpalettes.h>
#include <colorutils.h>
#include <controller.h>
#include <cpp_compat.h>
#include <dmx.h>
#include <FastLED.h>
#include <fastled_config.h>
#include <fastled_delay.h>
#include <fastled_progmem.h>
#include <fastpin.h>
#include <fastspi.h>
#include <fastspi_bitbang.h>
#include <fastspi_dma.h>
#include <fastspi_nop.h>
#include <fastspi_ref.h>
#include <fastspi_types.h>
#include <hsv2rgb.h>
#include <led_sysdefs.h>
#include <lib8tion.h>
#include <noise.h>
#include <pixelset.h>
#include <pixeltypes.h>
#include <platforms.h>
#include <power_mgt.h>


int const outputLEDPin=9;
float dutyCycle = 0;


class AnimationPong
{

private:
  int length;
  int numStrips;
  int numLeds;
  CRGB *leds;


  uint8_t  pongHue;
  uint16_t pongPosition;
  int      pongWidth;
  int      pongInc;
  int      pongBoundry;
  int      pongColorSwap = -1;


public:

  AnimationPong( 
    CRGB *leds,
    int ledStripLength,
    int numLedStrips
  );

  void Init( uint8_t hue )
  {
    pongHue      = hue;
    pongPosition = 0;
    pongWidth    = 4;
    pongInc      = 1;
    pongBoundry  = 30;
  }

  void Animation( float currentTime )
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
          leds[idx + pongBoundry * idxStrip].setHSV( pongHue + pongColorSwap * (int)50, 255, bightness );
        }
        else
        {
           leds[pongBoundry * (idxStrip+1)-idx-1].setHSV( pongHue - pongColorSwap * (int)50, 255, bightness );
        }
      }
    }

    pongPosition += pongInc;
    if ( pongPosition >= pongBoundry )
    {
      pongInc *= -1;
    }
    if (pongPosition == 0)
    {
      pongColorSwap *= -1;
      pongInc *= -1;
    }
  }
};






AnimationPong::AnimationPong( 
  CRGB *leds,
  int ledStripLength,
  int numLedStrips
)
{ 
  length        = ledStripLength;
  numStrips     = numLedStrips;
  numLeds       = ledStripLength * numLedStrips;
  leds          = leds;
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

  AnimationPong *pong;

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
    currentAnimation = LED_ANI_Solid;
    FastLED.addLeds< NEOPIXEL, outputLEDPin >( leds, numLeds );
    pong = new AnimationPong( leds, ledStripLength, numLedStrips );
  }

  ~LedAnimator()
  {
    delete []leds;
  }

  void Change_Speed(
    float newSpeed
  )
  {
      timeIncrement = newSpeed;
  }



  void Change_Animation(
    led_animation_t newAnimation
  )
  {
    currentAnimation = newAnimation;
    currentTime      = 0.0f;
    Init_Animation();
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
        //pong->Init( 0 );
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

  void  Update_Animation()
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
        //pong->Animation( currentTime );
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
      int hue = dutyCycle * 255.0f;
      leds[idx].setHSV( hue, 255, 255 );
    }
  }

  void Init_Rainbow( )
  {

  }

  void Animation_Rainbow( float currentTime )
  {
    uint8_t const offset = floor( currentTime ) * 3;

    for ( int idx = 0; idx < numLeds; idx++ )
    {
      uint8_t const hue = offset + idx*4;
      leds[idx].setHSV( hue, 255, 255 );
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
    float const brightArray[16] = {0.0, 0.1, 0.2, 0.3, 0.5, 0.7, 0.8, 0.9, 0.9, 0.8, 0.7, 0.5, 0.3, 0.2, 0.1, 0.0};
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

    vuTime += timeIncrement;
    if ( vuTime >= animationLen )
    {
       vuTime -= animationLen;
    }

    

    for ( int idx = 0; idx < vuLedLen; idx++ )
    {
      int const minPos = 0;
      int maxPos;
      uint8_t bightness = 100;
      uint8_t hue = 96;

      if (vuTime < (animationLen/2.0))
      {
        maxPos = ceil( (float)vuLedLen * (-0.25 * vuTime + 0.05 *vuTime * vuTime) / ((float)animationLen/2.0) );
      }
      else
      {
        float vuTime2 = ((float)animationLen) - vuTime;
        maxPos = ceil( (float)vuLedLen * (vuTime2 / ((float)animationLen/2.0) ));
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
        bightness = 255;
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
  int      pongColorSwap = -1;

  void Init_Pong( uint8_t hue )
  {
    pongHue      = hue;
    pongWidth    = 4;
    pongBoundry  = 60;
    TurnOffLeds();
  }

  void Animation_Pong( float inTime )
  {
    uint8_t const numStrips2 = (numLeds + pongBoundry) / pongBoundry - 1;

    float pongPosition = 0.0;


    if ( currentTime < ( pongBoundry * 1.0 ) )
    {
        pongPosition = currentTime;
    }
    else if ( currentTime < ( pongBoundry * 2.0 ) )
    {
        pongPosition = ( pongBoundry * 2.0 ) - currentTime;
    }
    else
    {
        currentTime = 0;
    }

    for ( int idx = 0; idx < pongBoundry; idx++ )
    {
      int const minPos = pongPosition - pongWidth;
      int const maxPos = pongPosition + pongWidth;
      uint8_t bightness = 0;

      if ( ( idx > minPos ) && ( idx < maxPos ) )
      {
        bightness = 255;
      }
      else
      {
        bightness = 0;
      }
      
      for ( int idxStrip = 0; idxStrip < numStrips2; idxStrip++ )
      {
        if (idxStrip % 2)
        {
          leds[idx + pongBoundry * idxStrip].setHSV( pongHue , 255, bightness );
        }
        else
        {
           leds[pongBoundry * (idxStrip+1)-idx-1].setHSV( pongHue, 255, bightness );
        }
      }
    }
  }
};






int const gbl_ledStripLength = 60;
int const gbl_numLedStrips   = 4;
int const ledDriverPin       = 9;


LedAnimator *ledDriver = NULL;





//***************************************************************************
//  
//
//
//***************************************************************************
void initializeLedDriver()
{
  ledDriver = new LedAnimator( gbl_ledStripLength, gbl_numLedStrips );
  ledDriver->Init_Animation();
}



//***************************************************************************
//  
//
//
//***************************************************************************
void updateLedDriver()
{
  ledDriver->Update_Animation();
}



//***************************************************************************
//  
//
//
//***************************************************************************
void setup()
{
  //Serial.begin(9600);
  initializeLedDriver();
}



unsigned long animationChangeTime = 0;
int currentAnimationIdx = 0;




//***************************************************************************
//  
//
//
//***************************************************************************
void loop()
{
  unsigned long const highTime           = pulseIn( 8, HIGH );
  unsigned long const lowTime            = pulseIn( 8, LOW );
  unsigned long const cycleTime          = highTime + lowTime;
  unsigned long const currentMillisecond = millis();
  float         const dutyCycle2         = (float)highTime / (float)cycleTime;

  dutyCycle = (((float)highTime / float(cycleTime)) - 0.48) * 1.9;

  if ( !( dutyCycle < 0.0f || dutyCycle > 1.0f ) )
  {
    ledDriver->Change_Speed( dutyCycle * 4.0f );
  }

  if ( ( dutyCycle < 0.0f || dutyCycle > 1.0f ) && 
       ( (currentMillisecond - animationChangeTime) > 250 ) )
  { 
     //Serial.print( dutyCycle2 * 255 );
     //Serial.print(" ");
     //Serial.print( dutyCycle );
     //Serial.print("\n");
     currentAnimationIdx = floor( 0.5 + ( dutyCycle2 * 255.0 ) / 10.0 );

     animationChangeTime = millis();
     //currentAnimationIdx++;
     //currentAnimationIdx %= NUM_LED_ANIMATIONS;
     ledDriver->Change_Animation( currentAnimationIdx );
  }


  updateLedDriver();
}
