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

public:

  using LedAnimationBase::LedAnimationBase;

  void Init()
  {
    for ( int idx = 0; idx < this->numLeds; idx++ )
    {
      this->leds[idx] = CRGB::Black;
    }
  }

  void Animation()
  {
    for ( int idx = 0; idx < this->numLeds; idx++ )
    {
      int hue = dutyCycle * 255.0f;
      this->leds[idx].setHSV( hue, 255, 255 );
    }
  }
};



//***************************************************************************
//  
//  AnimationSolidCycle
//
//  description: Solid color cycleing with time
//
//***************************************************************************
class AnimationSolidCycle : public LedAnimationBase
{

private:

public:

  using LedAnimationBase::LedAnimationBase;

  void Init()
  {
    for ( int idx = 0; idx < this->numLeds; idx++ )
    {
      this->leds[idx] = CRGB::Black;
    }
  }

  void Animation( float animationProgress )
  {
    for ( int idx = 0; idx < this->numLeds; idx++ )
    {
      int hue = animationProgress * 255.0f;
      this->leds[idx].setHSV( hue, 255, 255 );
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
      this->leds[idx].setHSV( hue, 255, 255 );
    }
  }
};



//***************************************************************************
//  
//  AnimationTeamColors
//
//  description: Team Colors
//
//***************************************************************************
class AnimationTeamColors : public LedAnimationBase
{

private:

public:

  using LedAnimationBase::LedAnimationBase;

  void Init()
  {
    for ( int idx = 0; idx < this->numLeds; idx++ )
    {
      this->leds[idx] = CRGB::Black;
    }
  }

  void Animation( float animationProgress )
  {
    //int cnt = 0;
    //Serial.print(animationProgress);
    //Serial.print("\n");
    //uint8_t idxColor = 0;
    for ( int idx = 0; idx < this->stripLength; idx++ )
    {
      float idx2        = animationProgress * 60.0 + idx;
      uint8_t idxColor  = (uint8_t)idx2%60;//round( idx2 / 4.0 / 4.0 ) % 4;
      uint8_t hue       = 0;
      uint8_t bightness = 255;
      uint8_t sat       = 255;

      if ( idx2 >= 60.0 )
      {
        idx2 -= 60.0f;
      }

      float rem = idx2 - idxColor;


      //cnt++;
      if ( idxColor < 1 )
      {
        hue = 35;
        bightness = 255 * rem;
      }
      else if ( idxColor < 14 )
      {
        hue = 35;
      }
      else if ( idxColor < 15 )
      {
        hue = 35;
        bightness = 255 * (1.0f-rem);
      }
      else if  ( idxColor < 30 )
      {
        bightness = 0;
      }
      else if  ( idxColor < 31 )
      {
        sat = 0;
        bightness = 255 * rem;
      }
      else if  ( idxColor < 44 )
      {
        sat = 0;
      }
      else if  ( idxColor < 45 )
      {
        sat = 0;
        bightness = 255 * (1.0f-rem);
      }
      else
      {
        bightness = 0;
      }

      for ( int idxStrip = 0; idxStrip < this->numStrips; idxStrip++ )
      {
        this->leds[idx + idxStrip * this->stripLength].setHSV( hue, sat, bightness );
      }
    }
  }
};



//***************************************************************************
//  
//  AnimationMurica
//
//  description:
//
//***************************************************************************
class AnimationMurica : public LedAnimationBase
{

private:

public:

  using LedAnimationBase::LedAnimationBase;

  void Init()
  {
    for ( int idx = 0; idx < this->numLeds; idx++ )
    {
      this->leds[idx] = CRGB::Black;
    }
  }

  void Animation( float animationProgress )
  {
    //int cnt = 0;
    //Serial.print(animationProgress);
    //Serial.print("\n");
    //uint8_t idxColor = 0;
    for ( int idx = 0; idx < this->stripLength; idx++ )
    {
      float   idx2        = idx;//animationProgress * 60.0 + idx;
      uint8_t idxColor  = ((uint8_t)idx2%60) / 4;//round( idx2 / 4.0 / 4.0 ) % 4;
      uint8_t hue       = 0;
      uint8_t bightness = 255 * dutyCycle;
      uint8_t sat       = 255;

      if ( idx2 >= 60.0 )
      {
        idx2 -= 60.0f;
      }

      float rem = idx2 - idxColor;

      if ( idxColor == 0 || idxColor == 14 )
      {
        bightness = 0;
      }
      else
      {
        if ( idxColor % 2 )
        {
          sat = 255;
        }
        else
        {
          sat = 0;
        }
      }

      for ( int idxStrip = 0; idxStrip < this->numStrips; idxStrip++ )
      {
        if ( idxStrip == 1 || idxStrip == 2 )
        {
          hue = 0;
          if ( !(idxColor == 0 || idxColor == 14) )
          {
            if ( idxColor % 2 )
            {
              sat = 255;
            }
            else
            {
              sat = 0;
            }
          }

          this->leds[idx + idxStrip * this->stripLength].setHSV( hue, sat, bightness );
        }
        else
        {
          if ( !(idxColor == 0 || idxColor == 14) )
          {
            if ( idxColor < 7 )
            {
              hue = 0;
              if ( idxColor % 2 )
              {
                sat = 255;
              }
              else
              {
                sat = 0;
              }
            }
            else
            {
              hue = 160;
              sat = 255;
            }
          }

          this->leds[idx + idxStrip * this->stripLength].setHSV( hue, sat, bightness );
        }
      }
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
          this->leds[idx + this->larsonBoundry * idxStrip].setHSV( hue , 255, bightness );
        }
        else
        {
           this->leds[this->larsonBoundry * (idxStrip+1)-idx-1].setHSV( hue, 255, bightness );
        }
      }
    }
  }
};

uint8_t gbl_ledProgress[240];
uint8_t gbl_ledColor[240];
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
  float *ledProgress;
  float ledProgressInc;
  float ledProgressMax;
  bool  randomColor;
public:
  uint16_t randMod;
  using LedAnimationBase::LedAnimationBase;

  void Init( bool randomColor, float ledProgressMax )
  {
      this->randMod        = 1000;
      this->ledProgressInc = 1;
      this->ledProgressMax = ledProgressMax;
      this->randomColor    = randomColor;
      srand( 0x1337 );

      for ( int idx = 0; idx < this->numLeds; idx++ )
      {
        gbl_ledProgress[idx] = 0;
      }
  }

  void Animation( float animationProgress )
  {
    for ( int idx = 0; idx < this->numLeds; idx++ )
    {
      if ( gbl_ledProgress[idx] == 0 )
      {
        if ( ( rand() % this->randMod ) == 0 )
        {
          gbl_ledProgress[idx] = this->ledProgressInc;
          gbl_ledColor[idx]    = rand() % 3;
        }
      }
      else if ( gbl_ledProgress[idx] >= this->ledProgressMax )
      {
        gbl_ledProgress[idx] = 0;
      }
      else
      {
        gbl_ledProgress[idx] += this->ledProgressInc;
      }
    }

    for ( int idx = 0; idx < this->numLeds; idx++ )
    {
      uint8_t hue = 0;
      uint8_t sat = 0;
      uint8_t bightness = 0;
      uint8_t mult = 256 / this->ledProgressMax;

      if ( gbl_ledProgress[idx] < ( this->ledProgressMax / 2 ) )
      {
        bightness = gbl_ledProgress[idx] * 2 * mult;
      }
      else
      {
        uint16_t progress;
        if ( gbl_ledProgress[idx] > this->ledProgressMax )
        {
          progress = this->ledProgressMax;
        }
        else
        {
          progress = gbl_ledProgress[idx];
        }
        bightness = mult * ( this->ledProgressMax - ( progress - ( this->ledProgressMax / 2 ) ) * 2 );
      }

      if ( this->randomColor )
      {
        if ( gbl_ledColor[idx] == 0 )
        {
          hue = 0 + dutyCycle * 256;
          sat = 255;   
        }
        else if ( gbl_ledColor[idx] == 1 )
        {
          hue = 170 + dutyCycle * 256;
          sat = 255;   
        }
        else
        {
          hue = 0;
          sat = 0;   
        }

      }

      this->leds[idx].setHSV( hue, sat, bightness );
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
          this->leds[idx + this->vuLedLen * idxStrip].setHSV( hue, 255, bightness );
        }
        else
        {
           this->leds[this->vuLedLen * (idxStrip+1)-idx - 1].setHSV( hue, 255, bightness );
        }
      }
    }
  }
};


typedef enum
{
  LED_ANI_None,
  LED_ANI_Solid,
  LED_ANI_SolidCycle,
  LED_ANI_Raindow,
  LED_ANI_Pong,
  LED_ANI_Vu,
  LED_ANI_LarsonScanner,
  LED_ANI_Starfield,
  LED_ANI_Confetti,
  LED_ANI_TeamColors,
  LED_ANI_Murica,
  NUM_LED_ANIMATIONS
} led_animation_t;




//CRGB gbl_leds[ 240 ];


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

  AnimationSolid         *solid;
  AnimationPong          *pong;
  AnimationLarsonScanner *larson;
  AnimationRainbow       *rainbow;
  AnimationVu            *vu;
  Starfield              *star;
  AnimationSolidCycle    *solidCycle;
  AnimationTeamColors    *teamColors;
  AnimationMurica        *murica;
public:



  LedAnimator( 
    int ledStripLength,
    int numLedStrips
  )
  { 
    this->length        = ledStripLength;
    this->numStrips     = numLedStrips;
    this->numLeds       = ledStripLength * numLedStrips;
    this->leds          = new CRGB[ this->numLeds ];     
    this->timeIncrement = 1.0f;
    this->currentTime   = 0.0f;
    this->timeFold      = 100.0f;
    this->currentAnimation = LED_ANI_Solid;

    FastLED.addLeds< NEOPIXEL, gbl_ledDriverPin >( this->leds, this->numLeds );
    this->solid      = new AnimationSolid( this->leds, this->length, this->numStrips );
    this->pong       = new AnimationPong( this->leds, this->length, this->numStrips );
    this->larson     = new AnimationLarsonScanner( this->leds, this->length, this->numStrips );
    this->rainbow    = new AnimationRainbow( this->leds, this->length, this->numStrips );
    this->vu         = new AnimationVu( this->leds, this->length, this->numStrips );
    this->star       = new Starfield( this->leds, this->length, this->numStrips );
    this->solidCycle = new AnimationSolidCycle( this->leds, this->length, this->numStrips ); 
    this->teamColors = new AnimationTeamColors( this->leds, this->length, this->numStrips ); 
    this->murica     = new AnimationMurica( this->leds, this->length, this->numStrips );
  }

  ~LedAnimator()
  {
    delete []this->leds;
  }

  void Change_Speed(
    float newSpeed
  )
  {
    if ( this->currentAnimation == LED_ANI_Starfield)
    {
      this->timeIncrement = 2;
      int newBoundry = floor( 1000 / pow(3, floor(newSpeed)));
      star->randMod = newBoundry;
    }
    else if ( currentAnimation == LED_ANI_Pong)
    {
      this->timeIncrement = 2;
      int newBoundry = floor(120.0 / pow(2.0, floor(newSpeed*1.5)));
      if ( pong->boundry != newBoundry )
      {
        pong->boundry = newBoundry;
      } 
    }
    else
    {
      this->timeIncrement = newSpeed;
    }
  }



  void Change_Animation(
    led_animation_t newAnimation
  )
  {
    this->currentAnimation = newAnimation;
    this->currentTime      = 0.0f;
    Init_Animation();
  }



  void Init_Animation()
  {
    switch ( this->currentAnimation )
    {
      case LED_ANI_Solid:
      {
        this->solid->Init();
        break;
      }

      case LED_ANI_SolidCycle:
      {
        this->solidCycle->Init();
        break;
      }

      case LED_ANI_Raindow:
      {
        this->rainbow->Init();
        break;
      }

      case LED_ANI_Pong:
      {
        this->pong->Init( 96 );
        break;
      }

      case LED_ANI_Vu:
      {
        this->vu->Init( );
        break;
      }

      case LED_ANI_LarsonScanner:
      {
        this->larson->Init();
        break;
      }

      case LED_ANI_Starfield:
      {
        this->star->Init( false, 60  );
        break;
      }

      case LED_ANI_Confetti:
      {
        this->star->Init( true, 15 );
        this->star->randMod = 30;
        break;
      }

      case LED_ANI_TeamColors:
      {
        this->teamColors->Init( );
        break;
      }

      case LED_ANI_Murica:
      {
        this->murica->Init( );
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

  void Update_Animation( )
  {
    this->currentTime += this->timeIncrement;
    if ( this->currentTime >= this->timeFold )
    {
      this->currentTime -= this->timeFold;
    }

    switch ( this->currentAnimation )
    {
      case LED_ANI_Solid:
      {
        this->solid->Animation( );
        break;
      }

      case LED_ANI_SolidCycle:
      {
        this->solidCycle->Animation( currentTime / timeFold );
        break;
      }

      case LED_ANI_Raindow:
      {
        this->rainbow->Animation( currentTime / timeFold );
        break;
      }

      case LED_ANI_Pong:
      {
        this->pong->Animation( currentTime / timeFold );
        break;
      }

      case LED_ANI_Vu:
      {
        this->vu->Animation( currentTime / timeFold );
        break;
      }

      case LED_ANI_LarsonScanner:
      {
        this->larson->Animation( currentTime / timeFold );
        break;
      }
      
      case LED_ANI_Starfield:
      {
        this->star->Animation( currentTime / timeFold );
        break;
      }

      case LED_ANI_Confetti:
      {
        this->star->Animation( currentTime / timeFold );
        break;
      }

      case LED_ANI_TeamColors:
      {
        this->teamColors->Animation( currentTime / timeFold );
        break;
      }

      case LED_ANI_Murica:
      {
        this->murica->Animation( currentTime / timeFold );
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
    for ( int idx = 0; idx < this->numLeds; idx++ )
    {
      this->leds[idx] = CRGB::Black;
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


int dutyCycleMeasCnt = 0;

uint8_t const numAnimations = 11;
uint8_t newAnimationIdxCnt[numAnimations] = { 0 };


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
    dutyCycleMeasCnt = 0;
  }

  if ( ( dutyCycle < 0.0f || dutyCycle > 1.0f ) )
  { 
    bool newAnimationFound = false;
    int newAnimationIdx = floor( 0.5 + ( dutyCycle2 * 255.0 ) / 10.0 );  

    dutyCycleMeasCnt++;
    if ( newAnimationIdx < numAnimations )
    {
      newAnimationIdxCnt[newAnimationIdx]++;
    }

    for ( int idx = 0; idx < numAnimations; idx++ )
    {
      if ( newAnimationIdxCnt[idx] > 2 )
      {
        newAnimationIdx   = idx;
        newAnimationFound = true;
        break;
      }
    }

    if ( newAnimationFound == true )
    {
      if ( newAnimationIdx != currentAnimationIdx )
      {
        ledDriver->Change_Animation( newAnimationIdx );
        currentAnimationIdx = newAnimationIdx;
      }
      dutyCycleMeasCnt = 0;
      for ( int idx = 0; idx < numAnimations; idx++ )
      {
        newAnimationIdxCnt[idx] = 0;
      }
    }
  }


  updateLedDriver();
}
