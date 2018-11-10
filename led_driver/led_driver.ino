#include <FastLED.h>

int const gbl_ledStripLength = 60;
int const gbl_numLedStrips   = 4;
int const gbl_ledDriverPin   = 9;


float         dutyCycle           = 0;
unsigned long animationChangeTime = 0;
int           currentAnimationIdx = 0;



//***************************************************************************
//  
//  LedAnimationBase
//
//  description: Base class for creating LED animations.
//               Contains the LED strip information.
//
//***************************************************************************
class LedAnimationBase
{

private:

public:

  int  stripLength;
  int  numStrips;
  int  numLeds;
  CRGB *leds;


  LedAnimationBase( 
    CRGB *leds,
    int  stripLength,
    int  numStrips
  )
  { 
    this->stripLength = stripLength;
    this->numStrips   = numStrips;
    this->numLeds     = stripLength * numStrips;
    this->leds        = leds;
  }
};




//***************************************************************************
//  
//  AnimationSolid
//
//  description: Solid color
//
//***************************************************************************
class AnimationSolid : public LedAnimationBase
{

private:
  float animationLen;
  int   vuLedLen;

public:

  using LedAnimationBase::LedAnimationBase;

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


};




//***************************************************************************
//  
//  AnimationPong
//
//  description: Moving Rainbow animation.
//
//***************************************************************************
class AnimationRainbow : public LedAnimationBase
{

private:

public:

  using LedAnimationBase::LedAnimationBase;

  void Init( )
  {

  }

  void Animation( float animationProgress )
  {
    uint8_t const offset = animationProgress * 255;

    for ( int idx = 0; idx < this->numLeds; idx++ )
    {
      uint8_t const hue = offset + idx * 4;
      leds[idx].setHSV( hue, 255, 255 );
    }
  }
};



//***************************************************************************
//  
//  AnimationPong
//
//  description: Pong like animation.
//
//***************************************************************************
class AnimationPong : public LedAnimationBase
{

private:
  uint8_t  hue;
  int      width;


public:
  int      boundry;
  using LedAnimationBase::LedAnimationBase;

  void Init( uint8_t hue )
  {
    this->hue      = hue;
    this->width    = 1;
    this->boundry  = 30;
  }

  void Animation( float animationProgress )
  {
    uint8_t const numStrips2        = (this->numLeds + this->boundry) / this->boundry - 1;
    float   const timeIntoAnimation = animationProgress * this->boundry;

    float pongPosition = 0.0;


    if ( animationProgress < 0.5 )
    {
        pongPosition = animationProgress * this->boundry * 2;
    }
    else
    {
        pongPosition = ( 1.0f - animationProgress ) * this->boundry * 2;
    }

    for ( int idx = 0; idx < this->boundry; idx++ )
    {
      int const minPos = floor(pongPosition - this->width);
      int const maxPos = ceil(pongPosition + this->width);
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
           this->leds[idx + this->boundry * idxStrip].setHSV( this->hue , 255, bightness );
        }
        else
        {
           this->leds[this->boundry * (idxStrip+1)-idx-1].setHSV( this->hue, 255, bightness );
        }
      }
    }
  }
};




//***************************************************************************
//  
//  AnimationLarsonScanner
//
//  description: 
//
//***************************************************************************
class AnimationLarsonScanner : public LedAnimationBase
{

private:
  int larsonWidth;
  int larsonBoundry;

public:

  using LedAnimationBase::LedAnimationBase;

  void Init( )
  {
    larsonWidth   = 4;
    larsonBoundry = 60;
  }

  void Animation( float animationProgress )
  {
    // Larson scanner is always red
    uint8_t const hue = 0;
    uint8_t const numStrips2 = (this->numLeds + this->larsonBoundry) / this->larsonBoundry - 1;

    float position = 0.0;


    if ( animationProgress < 0.5 )
    {
        position = animationProgress * this->larsonBoundry * 2;
    }
    else
    {
        position = ( 1.0f - animationProgress ) * this->larsonBoundry * 2;
    }

    for ( int idx = 0; idx < this->larsonBoundry; idx++ )
    {
      int const minPos = position - this->larsonWidth;
      int const maxPos = position + this->larsonWidth;
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
          leds[idx + this->larsonBoundry * idxStrip].setHSV( hue , 255, bightness );
        }
        else
        {
           leds[this->larsonBoundry * (idxStrip+1)-idx-1].setHSV( hue, 255, bightness );
        }
      }
    }
  }
};


//***************************************************************************
//  
//  AnimationStarfield
//
//  description: random twinkling lights
//
//***************************************************************************
class Starfield : public LedAnimationBase
{

private:


public:

  using LedAnimationBase::LedAnimationBase;

  void Init( )
  {
    
  }

  void Animation( float animationProgress )
  {
    uint8_t bightness=255;
    if ( animationProgress < 0.5 )
    {
      bightness = animationProgress*2 * 250;
    }
    else
    {
      bightness = (1-(animationProgress-0.5)*2) * 250;
    }

    for ( int idx = 0; idx < this->numLeds; idx++ )
    {
      leds[idx].setHSV( 0, 0, bightness );// hue, saturation, bightness
    }
  }
};







//***************************************************************************
//  
//  AnimationVu
//
//  description: VU Meter
//
//***************************************************************************
class AnimationVu : public LedAnimationBase
{

private:
  float animationLen;
  int   vuLedLen;

public:

  using LedAnimationBase::LedAnimationBase;

  void Init( )
  {
    this->animationLen = 35;
    this->vuLedLen     = 60;
  }

  void Animation( float animationProgress )
  {
    uint8_t const numStrips2 = (numLeds + this->vuLedLen) / this->vuLedLen - 1;

    float const vuTime = animationProgress * this->animationLen;
    

    for ( int idx = 0; idx < this->vuLedLen; idx++ )
    {
      int const minPos = 0;
      int maxPos;
      uint8_t bightness = 100;
      uint8_t hue = 96;

      if (vuTime < (this->animationLen/2.0))
      {
        maxPos = ceil( (float)this->vuLedLen * (-0.25 * vuTime + 0.05 *vuTime * vuTime) / 
                       ((float)this->animationLen/2.0) );
      }
      else
      {
        float vuTime2 = ((float)this->animationLen) - vuTime;
        maxPos = ceil( (float)this->vuLedLen * (vuTime2 / ((float)this->animationLen/2.0) ));
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
          leds[idx + this->vuLedLen * idxStrip].setHSV( hue, 255, bightness );
        }
        else
        {
           leds[this->vuLedLen * (idxStrip+1)-idx - 1].setHSV( hue, 255, bightness );
        }
      }
    }
  }
};


typedef enum
{
  LED_ANI_None,
  LED_ANI_Solid,
  LED_ANI_Raindow,
  LED_ANI_Pong,
  LED_ANI_Vu,
  LED_ANI_Rain,
  LED_ANI_LarsonScanner,
  LED_ANI_Starfield,
  NUM_LED_ANIMATIONS
} led_animation_t;







//***************************************************************************
//  
//  LedAnimator
//
//  description: Class for running animations on an LED strip.
//
//***************************************************************************
class LedAnimator
{

private:
  int length;
  int numStrips;
  int numLeds;
  CRGB *leds;
  float timeIncrement;
  float currentTime;
  float timeFold;
  led_animation_t currentAnimation;

  AnimationPong          *pong;
  AnimationLarsonScanner *larson;
  AnimationRainbow       *rainbow;
  AnimationVu            *vu;
  Starfield              *star;
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
    timeFold      = 100.0f;
    currentAnimation = LED_ANI_Starfield;

    FastLED.addLeds< NEOPIXEL, gbl_ledDriverPin >( leds, numLeds );
    pong    = new AnimationPong( leds, ledStripLength, numLedStrips );
    larson  = new AnimationLarsonScanner( leds, ledStripLength, numLedStrips );
    rainbow = new AnimationRainbow( leds, ledStripLength, numLedStrips );
    vu      = new AnimationVu( leds, ledStripLength, numLedStrips );
    star    = new Starfield( leds, ledStripLength, numLedStrips );
  }

  ~LedAnimator()
  {
    delete []leds;
  }

  void Change_Speed(
    float newSpeed
  )
  {
    if ( currentAnimation == LED_ANI_Pong)
    {
      timeIncrement = 2;
      int newBoundry = floor(120.0 / pow(2.0, floor(newSpeed*1.5)));
      if ( pong->boundry != newBoundry )
      {
        pong->boundry = newBoundry;
      } 
    }
    else
    {
      timeIncrement = newSpeed;
    }
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
        rainbow->Init();
        break;
      }

      case LED_ANI_Pong:
      {
        pong->Init( 96 );
        break;
      }

      case LED_ANI_Vu:
      {
        vu->Init( );
        break;
      }

      case LED_ANI_Rain:
      {
        Init_Rain( );
        break;
      }

      case LED_ANI_LarsonScanner:
      {
        larson->Init();
        break;
      }

      case LED_ANI_Starfield:
      {
        star->Init();
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
    if ( currentTime >= timeFold )
    {
      currentTime -= timeFold;
    }

    switch ( currentAnimation )
    {
      case LED_ANI_Solid:
      {
        Animation_Solid();
        break;
      }

      case LED_ANI_Raindow:
      {
        rainbow->Animation( currentTime / timeFold );
        break;
      }

      case LED_ANI_Pong:
      {
        pong->Animation( currentTime / timeFold );
        break;
      }

      case LED_ANI_Vu:
      {
        vu->Animation( currentTime / timeFold );
        break;
      }
      
      case LED_ANI_Rain:
      {
        Animation_Rain( currentTime );
        break;
      }

      case LED_ANI_LarsonScanner:
      {
        larson->Animation( currentTime / timeFold );
        break;
      }
      
      case LED_ANI_Starfield:
      {
        star->Animation( currentTime / timeFold );
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
};




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
