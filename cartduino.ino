//Andrew Panning 9/29/2018

#include <ezLCDLib.h>
#include <DS3231.h>
#include <Wire.h>

#define LED_PIN 13
#define METERDIGITS 2
#define METERDP 0
#define LEDX 175
#define LEDY 70

DS3231 Clock;//Clock object to set time
ezLCD3 lcd;//LCD object
RTClib RTC;//RTC object
int touch=0;//records touched element on LCD
int sliderval=125;
unsigned long currentMillis=0;
unsigned long previousMillis=0;
bool updatehour=0;
int lastloophour=99;
int adjusthour=99;

int lastloopsecond=99;
float sum = 0.0;// sum of samples taken
int sample_count = 0;// sample number
float voltage = 0.0;// calculated voltage

void setup()
{
  lcd.begin( EZM_BAUD_RATE );
  lcd.cls( BLACK, WHITE );
  lcd.font("0");
  lcd.printString("Fridley Tiger Robotics");
  lcd.print("\\[20y");
  lcd.print("\\[0x");
  lcd.printString("#2227");
  lcd.fontw( 1, "LCD24" );
  lcd.theme( 1, 1, 3, BLACK, BLACK, BLACK, ORANGE, YELLOW, 1, 1, 1 );//theme 1
  lcd.string( 1, "HORN" ); // stringId 1
  lcd.button( 1, 210, 55, 100, 80, 1, 0, 20, 1, 1 );
  lcd.drawLed( LEDX, LEDY, 12, BLACK, WHITE);
  lcd.theme(5, 1, 2, 0, 3, 3, 4, 4, 5, 6, 0);// theme 5
  lcd.digitalMeter( 5, 150, 100, 50, 30, 14, 0, 3, METERDP, 5);//draw digital meter 5
  lcd.theme(3, 1, 2, 0, 3, 0, 6, 4, 23, 35, 1);// theme 3
  lcd.string( 3, "Lights" ); // stringId 3
  lcd.string( 4, "Invert" ); // stringId 4
  lcd.string( 6, "Radio" ); // stringId 6
  //lcd.checkbox( ID, x, y, width, height, option, theme, string);
  lcd.checkBox( 3, 1, 60, 130, 50, 1, 3, 3 );
  lcd.checkBox( 4, 1, 120, 130, 50, 1, 3, 4 );
  lcd.checkBox( 6, 1, 180, 130, 50, 1, 3, 6 );
  lcd.theme(2, 2, 3, 3, 3, 3, 35, 6, 2, 0, 1);//theme 2
  lcd.slider( 2, 150, 145, 160, 40, 5, 125, 1, 0, 2 );
  lcd.digitalMeter( 7, 230, 200, 30, 30, 14, 0, METERDIGITS, METERDP, 5);//draw digital meter 7, with theme of 5
  lcd.digitalMeter( 8, 190, 200, 30, 30, 14, 0, METERDIGITS, METERDP, 5);//draw digital meter 8, with theme of 5
  lcd.digitalMeter( 9, 150, 200, 30, 30, 14, 0, METERDIGITS, METERDP, 5);//draw digital meter 9, with theme of 5
  lcd.theme(10, 0, 0, 0, 1, 2, 1, 0, 0, 6, 1);
  lcd.string( 10, "XX" ); // stringId 10
  lcd.string( 11, "PM" ); // stringId 11
  lcd.string( 12, "AM" ); // stringId 12
  lcd.staticText( 10, 270, 200, 30, 30, 4, 10, 10 );
  lcd.digitalMeter( 13, 200, 10, 60, 30, 14, 0, 4, 2, 5);//draw digital meter 13, with theme of 5
  lcd.string( 14, "VDC" ); // stringId 14
  lcd.staticText( 14, 270, 10, 45, 30, 4, 10, 14 );

  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode( LED_PIN, OUTPUT );
  pinMode(9, OUTPUT);//490Hz PWM (2040.8us period)
  digitalWrite( 4, HIGH );
  digitalWrite( 5, HIGH );
  digitalWrite( 6, HIGH );
  digitalWrite( 7, HIGH );
  digitalWrite( LED_PIN, LOW );
  analogWrite(9,125);//analogWrite values from 0 to 255,

  //RTC begin communication
  Wire.begin();
  if(0){ //set time
    Clock.setYear(2018);
    Clock.setMonth(9);
    Clock.setDate(23);
    Clock.setDoW(1);
    Clock.setHour(8+12);
    Clock.setMinute(39);
    Clock.setSecond(0);
  }
}






void loop()
{
  touch = lcd.wstack(0);
  if( touch ==1 ) {// if Button 1 is pressed:
    if( lcd.currentInfo ==4 ){
      digitalWrite( 5, LOW );
      lcd.drawLed( LEDX, LEDY, 12, RED, WHITE);
    }else {// if button 1 is released
      digitalWrite( 5, HIGH );
      lcd.drawLed( LEDX, LEDY, 12, BLACK, WHITE);
    }    
  }
  if( touch ==3 ) { //if checkbox 3 was pressed:
    if( lcd.currentInfo ==4 )     // if checkbox 3 is checked
      digitalWrite( 4, LOW );
    else  // if checkbox 1 is unchecked
      digitalWrite( 4, HIGH );      
  }
  if( touch ==4 ) { //if checkbox 4 was pressed:
    if( lcd.currentInfo ==4 )     // if checkbox 4 is checked
      digitalWrite( 6, LOW );
    else  // if checkbox 1 is unchecked
      digitalWrite( 6, HIGH );      
  }
  if( touch ==6 ) { //if checkbox 6 was pressed:
    if( lcd.currentInfo ==4 )     // if checkbox 6 is checked
      digitalWrite( 7, LOW );
    else  // if checkbox 1 is unchecked
      digitalWrite( 7, HIGH );      
  }


  //set pwm ou pin 9 for Blinkin
  sliderval=125+lcd.wvalue(2);
  lcd.wvalue(5, sliderval);
  currentMillis = millis();
  if(currentMillis - previousMillis > 100){//only update pwm every 100 milliseconds
    previousMillis=currentMillis;
    analogWrite(9,sliderval);
  }
  
  





  //read and display RTC
  DateTime now = RTC.now();
  lcd.wvalue(7, now.second());
  lcd.wvalue(8, now.minute());


  if(lastloophour!=now.hour()){
    lastloophour=now.hour();
    updatehour=1;
  }
  if(updatehour==1){
    updatehour=0;
     if(now.hour()>12)
    {
      adjusthour=now.hour()-12;
      lcd.st_value( 10, 11 );//PM
    }
  if(now.hour()==0){
      adjusthour=12;
      lcd.st_value( 10, 12 );//AM
    }
  if(now.hour()<=12 && now.hour()!=0){
      adjusthour=now.hour();
      lcd.st_value( 10, 12 );//AM
    }
  }
 
  lcd.wvalue(9, adjusthour);

  //read battery voltage with 1M & 100k resistor 11 voltage divider
  if(lastloopsecond!=now.second()){
    lastloopsecond=now.second();
    voltage=(float)sum/(float)sample_count;
    voltage=voltage*11.243;//scales x11 due to voltage divider, measured x11.243
    voltage=voltage*100;//scales for dp on display
    sample_count = 0;
    sum = 0;
  }
  sum += ((analogRead(A0)/1024.0*5.0)-0.181);//arduino reads 0.181V high
  sample_count++;
  lcd.wvalue(13, voltage);
  
  //slow execution delay before next loop
  delay(10);
}
