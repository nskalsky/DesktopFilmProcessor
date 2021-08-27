int PumpControl1 = 6;
int PumpControl2 = 7;

int SW = 62;
int DX = 63;
int FX = 64 ;

int ValveIO[] = {0, 5, 4, 2, 3};

#define READYTOSTARTMODE 0;
#define PUMPMODE 1;
#define AGITATEMODE 2;
#define WAITMODE 3;
#define CYCLECOMPLETEMODE 4;
#define CLEANMODE 5;

char ProcedureName[ ] = "E5 Black & White";
char* ModeStrings[]={"Ready..", "Pumping..", "Agitating..",
"Waiting..", "Cycle Complete..","Cleaning Manifold.."};
int ActiveMode = 0;

enum read_status {
  RX_START,
  RX_READ,
  RX_DONE
};

int DeveloperDirection = 1;
int ResevoirDirection = -1;
int primeTimeMs = 2500;
long TimeMsPerHundredMl = 6600;
long PumpRatePerTwoFifty = (TimeMsPerHundredMl/100)/4;

// Meter colour schemes
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5
#define WHITE2WHITE 6

int ActiveDevColor = 0;

// #include <avr/dtostrf.h> // Only needed for Due
#include <TFT_HX8357.h> // Hardware-specific library
TFT_HX8357 tft = TFT_HX8357();       // Invoke custom library

#define HX8357_GREY 0x2104 // Dark grey 16 bit colour


uint32_t runTime = -99999;       // time for next update

int reading = 0; // Value to be displayed
int d = 0; // Variable used for the sinewave test waveform
boolean alert = 0;
int8_t ramp = 1;
int xpos = 10, ypos = 10, gap = 8, radius = 62;

#include <TimeLib.h>


float TankVolume[] = {0, 0, 0, 0, 0};

void setup(void) {
  tft.begin();
  Serial.begin(9600);
  tft.setRotation(1);
  tft.fillScreen(HX8357_BLACK);
  initIO();
  UpdateUIGauges(0);
  TankVolume[0] = 0;
  TankVolume[1] = 600;
  TankVolume[2] = 600; 
  TankVolume[3] = 600;
  ActiveMode = READYTOSTARTMODE;

  /*
  ActivateValve(1);
  delay(2000);
  ActivateValve(2);
  delay(2000);
  ActivateValve(3);
  delay(2000);
  ActivateValve(4);

  digitalWrite(9,HIGH); 
  ActivateValve(0);
  ActivateValve(1);
  delay(2000);
  ActivateValve(0);
  ActivateValve(2);
  
  delay(2000);
  digitalWrite(9,LOW); 
  ActivateValve(0);
  ActivateValve(3);
 
    delay(2000);
   ActivateValve(0);
  ActivateValve(4);

  delay(2000);
  digitalWrite(9,HIGH); 
   ActivateValve(0);
    ActivateValve(1);
  delay(2000);
   ActivateValve(0);
  ActivateValve(2);
  delay(2000);
   ActivateValve(0);
   
  ActivateValve(3);
digitalWrite(9,LOW); 
    delay(2000);
     ActivateValve(0);
  ActivateValve(4);
*/

  
  //MoveFluidTime(DeveloperDirection,3,primeTimeMs+(TimeMsPerHundredMl*1));
  //delay(5000); 
  //MoveFluidTime(ResevoirDirection,2,TimeMsPerHundredMl*2);
 // BlackandWhiteDevProtocol();
 //PurgeDevTanks();
  TestTank(1);
}

void delayUILoop(long seconds){
  setTime(0);
  while (now() < seconds){
    UpdateUIGauges(seconds-now());
    delay(100);
    
  }
}

void delayUILoop(long seconds, int tankID, int destinationID){
  setTime(0);
  while (now() < seconds){
    UpdateUIGauges(seconds-now());
    delay(250);
    if (destinationID > 0){
        TankVolume[tankID] = 600 - (now() * 15); //TankVolume[tankID] - 5;
        TankVolume[0] = now() * 15;   //Dev tank
      } else {
        TankVolume[tankID] = now() * 15;
        TankVolume[0] = 600 - (now() * 15); //TankVolume[0] - 5;   //Dev tank
      } 
  }

  if (destinationID > 0){
    TankVolume[0] = 600;
    TankVolume[tankID] = 0; 
  } else {
    TankVolume[0] = 0;
    TankVolume[tankID] = 600; 
  }
  UpdateUIGauges(seconds-now());
  
}

void TestTank(int tankID){
  delay(2000);
  
  for (int i=0; i<11; i++){
    ActiveMode = PUMPMODE;
    MoveFluidTime(DeveloperDirection,tankID,primeTimeMs+(TimeMsPerHundredMl*3));
    ActiveMode = WAITMODE;
    delayUILoop(10); // 9min = 540
    ActiveMode = PUMPMODE;
    MoveFluidTime(ResevoirDirection,tankID,primeTimeMs+(TimeMsPerHundredMl*4));  
    ActiveMode = WAITMODE;
    delayUILoop(10); // 9min = 540
  }
  
  ActiveMode = CYCLECOMPLETEMODE;
  
}

void PurgeDevTanks(){
  MoveFluidTime(DeveloperDirection,1,primeTimeMs+(TimeMsPerHundredMl*6));
  MoveFluidTime(DeveloperDirection,2,primeTimeMs+(TimeMsPerHundredMl*6));
  MoveFluidTime(DeveloperDirection,3,primeTimeMs+(TimeMsPerHundredMl*6));
  
}
  
void BlackandWhiteDevProtocol(){
  //check pre-protocol parameters volumes
  delay(5000);
  //***** Step 1 Dev *****
  ActiveMode = PUMPMODE;
  MoveFluidTime(DeveloperDirection,1,primeTimeMs+(TimeMsPerHundredMl*7));
  ActiveMode = WAITMODE;
  delayUILoop(10); // 9min = 540
  ActiveMode = PUMPMODE;
  MoveFluidTime(ResevoirDirection,1,primeTimeMs+(TimeMsPerHundredMl*9));
  ActiveMode = CLEANMODE;
  QuickClean(2); // flush internal lines to reduce cross-contaimination

  //***** Step 2 Stop *****
  ActiveMode = PUMPMODE;
  MoveFluidTime(DeveloperDirection,2,primeTimeMs+(TimeMsPerHundredMl*6));
  ActiveMode = WAITMODE;
  delayUILoop(30); // 30 sec
  ActiveMode = PUMPMODE;
  MoveFluidTime(ResevoirDirection,2,primeTimeMs+(TimeMsPerHundredMl*7));
  ActiveMode = CLEANMODE;
  QuickClean(2); // flush internal lines to reduce cross-contaimination

  //***** Step 3 Fix *****
  ActiveMode = PUMPMODE;
  MoveFluidTime(DeveloperDirection,3,primeTimeMs+(TimeMsPerHundredMl*6));
  ActiveMode = WAITMODE;
  delayUILoop(30); // 5 min (300 sec)
  ActiveMode = PUMPMODE;
  MoveFluidTime(ResevoirDirection,3,primeTimeMs+(TimeMsPerHundredMl*9));
  ActiveMode = CLEANMODE;
  QuickClean(2); // flush internal lines to reduce cross-contaimination

   //***** Step 4 Rinse *****
  ActiveMode = PUMPMODE;
  MoveFluidTime(DeveloperDirection,4,primeTimeMs+(TimeMsPerHundredMl*6));
  ActiveMode = WAITMODE;
  delayUILoop(2);
  ActiveMode = PUMPMODE;
  MoveFluidTime(ResevoirDirection,4,primeTimeMs+(TimeMsPerHundredMl*7));
  ActiveMode = PUMPMODE;
  MoveFluidTime(DeveloperDirection,4,primeTimeMs+(TimeMsPerHundredMl*6));
  ActiveMode = WAITMODE;
  delayUILoop(2); 
  ActiveMode = PUMPMODE;
  MoveFluidTime(ResevoirDirection,4,primeTimeMs+(TimeMsPerHundredMl*7));

  //****** Done! ********
  ActiveMode = CYCLECOMPLETEMODE;
  UpdateUIGauges(0);
}

void UpdateUIGauges(int TimeRemaining){

  //tft.fillScreen(HX8357_BLACK);
  ringMeter(TankVolume[1],0,600, 30,10,radius,"  1 (ml)",RED2RED); // Draw analogue meter
  ringMeter(TankVolume[2],0,600, 180,10,radius,"  2 (ml)",GREEN2GREEN); // Draw analogue meter
  ringMeter(TankVolume[3],0,600, 320,10,radius,"  3 (ml)",BLUE2BLUE); // Draw analogue meter
  ringMeter(TankVolume[0],0,700, 50,145,75," Dev (ml)",ActiveDevColor); // Draw analogue meter

  tft.setCursor(250, 150, 2);
  tft.setTextFont(4);
  //tft.setTextColor(TFT_BLUE,TFT_BLACK);
  tft.print(ProcedureName);

  tft.setCursor(250, 180, 2);
  tft.setTextFont(4);
  //tft.setTextColor(TFT_BLUE,TFT_BLACK);
  tft.print(ModeStrings[ActiveMode]);
  
  tft.setCursor(250, 210, 2);
  tft.setTextColor(TFT_RED,TFT_BLACK);
  tft.setTextFont(4);
  tft.print("R");
  printDigits(TimeRemaining/60);
  printDigits(TimeRemaining%60);
  /*
  tft.setCursor(250, 270, 2);
  tft.setTextColor(TFT_GREEN,TFT_BLACK);
  tft.setTextFont(4);
  tft.print("Thermo-Stable");

  tft.setCursor(325, 150, 2);
   tft.setTextFont(4);
  //tft.setTextColor(TFT_BLUE,TFT_BLACK);
  tft.print("E: ");
  printDigits(minute());
  printDigits(second());
  //tft.println();*/
}

void initIO(){
  pinMode(PumpControl1, OUTPUT);
  pinMode(PumpControl2, OUTPUT);
  pinMode(ValveIO[1], OUTPUT);
  pinMode(ValveIO[2], OUTPUT); 
  pinMode(ValveIO[3], OUTPUT);
  pinMode(ValveIO[4], OUTPUT); 

 
  
  digitalWrite(ValveIO[1],LOW);
  digitalWrite(ValveIO[2],LOW);
  digitalWrite(ValveIO[3],HIGH);
  digitalWrite(ValveIO[4],HIGH);
  
}

void loop() {  

  
}

int ConvertMltoMs(int targetMl){ 
  return targetMl;
}

void QuickClean(int cycleCount){
  for (int i=0; i<cycleCount; i++){
      MoveFluidTime(DeveloperDirection,4,primeTimeMs*2);
      MoveFluidTime(ResevoirDirection,4,primeTimeMs*4);
  }
}

void ActivatePump(int Direction){
  if (Direction > 0){
    Serial.println("Sending fluid to developer tank");
    digitalWrite(PumpControl1,HIGH); // high = send fluid to developer
    digitalWrite(PumpControl2,LOW); //high = send fluid back to resevoir
  } else{
    digitalWrite(PumpControl1,LOW); // high = send fluid to developer
    digitalWrite(PumpControl2,HIGH); //high = send fluid back to resevoir
    Serial.println("Sending fluid back to holding tank");
  }  
}

void StopPump(){
    digitalWrite(PumpControl1,LOW); // high = send fluid to developer
    digitalWrite(PumpControl2,LOW); //high = send fluid back to resevoir
}

void ActivateValve(int valveID){ 
  digitalWrite(ValveIO[1],LOW); 
  digitalWrite(ValveIO[2],LOW); 
  digitalWrite(ValveIO[3],HIGH); 
  digitalWrite(ValveIO[4],HIGH); 
  if (valveID > 0 && valveID < 3){
    digitalWrite(ValveIO[valveID],HIGH); 
  } else if (valveID > 2){
    digitalWrite(ValveIO[valveID],LOW); 
  }
}

void MoveFluidTime(int destination, int tank,long TimeMS){
  switch (tank){
    case 0:
    case 1: 
        Serial.println("Activating Valve: Tank 1");
        ActiveDevColor = RED2RED;
        ActivateValve(1);
        delay(1000);
        Serial.print("Activating Pump: Calibrated Duration = ");
        Serial.print(TimeMS);
        Serial.println("ms");
        ActivatePump(destination);
        delayUILoop(TimeMS/1000,1,destination);//delay(TimeMS);
        StopPump();
        ActivateValve(0); // turn off all valves
        break;
    case 2:
        Serial.println("Activating Valve: Tank 2");
        ActiveDevColor = GREEN2GREEN;
        ActivateValve(2);
        delay(1000);//delay(500);
        ActivatePump(destination);
        delayUILoop(TimeMS/1000,2,destination);//delay(TimeMS);
        StopPump();
        ActivateValve(0);
        break;
    case 3:
        Serial.println("Activating Valve: Tank 3");
        ActiveDevColor = BLUE2BLUE;
        ActivateValve(3);
        delay(1000);//delay(500);
        ActivatePump(destination);
        delayUILoop(TimeMS/1000,3,destination);//delay(TimeMS);
        StopPump();
        ActivateValve(0);
        break;
    case 4: 
        Serial.println("Activating Valve: Tank 4");
        ActiveDevColor = WHITE2WHITE;
        ActivateValve(4);
        delay(1000);//delay(500);
        ActivatePump(destination);
        delayUILoop(TimeMS/1000,4,destination);//delay(TimeMS);
        StopPump();
        ActivateValve(0);
        break;
  }
}

// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################
int ringMeter(int value, int vmin, int vmax, int x, int y, int r, char *units, byte scheme)
{
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option
  
  x += r; y += r;   // Calculate coords of centre of ring

  int w = r / 3;    // Width of outer ring is 1/4 of radius
  
  int angle = 150;  // Half the sweep angle of meter (300 degrees)

  int v = map(value, vmin, vmax, -angle, angle); // Map the value to an angle v

  byte seg = 3; // Segments are 3 degrees wide = 100 segments for 300 degrees
  byte inc = 6; // Draw segments every 3 degrees, increase to 6 for segmented ring

  // Variable to save "value" text colour from scheme and set default
  int colour = HX8357_BLUE;
 
  // Draw colour blocks every inc degrees
  for (int i = -angle+inc/2; i < angle-inc/2; i += inc) {
    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (r - w) + x;
    uint16_t y0 = sy * (r - w) + y;
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * 0.0174532925);
    float sy2 = sin((i + seg - 90) * 0.0174532925);
    int x2 = sx2 * (r - w) + x;
    int y2 = sy2 * (r - w) + y;
    int x3 = sx2 * r + x;
    int y3 = sy2 * r + y;

    if (i < v) { // Fill in coloured segments with 2 triangles
      switch (scheme) {
        case 0: colour = HX8357_RED; break; // Fixed colour
        case 1: colour = HX8357_GREEN; break; // Fixed colour
        case 2: colour = HX8357_BLUE; break; // Fixed colour
        case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break; // Full spectrum blue to red
        case 4: colour = rainbow(map(i, -angle, angle, 70, 127)); break; // Green to red (high temperature etc)
        case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break; // Red to green (low battery etc)
        case 6: colour = HX8357_WHITE; break; // Fixed colour
        default: colour = HX8357_BLUE; break; // Fixed colour
      }
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      //text_colour = colour; // Save the last colour drawn
    }
    else // Fill in blank segments
    {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, HX8357_GREY);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, HX8357_GREY);
    }
  }
  // Convert value to a string
  char buf[10];
  byte len = 3; if (value > 999) len = 5;
  dtostrf(value, len, 0, buf);
  buf[len] = ' '; buf[len+1] = 0; // Add blanking space and terminator, helps to centre text too!
  // Set the text colour to default
  tft.setTextSize(1);

  if (value<vmin || value>vmax) {
    drawAlert(x,y+90,50,1);
  }
  else {
    drawAlert(x,y+90,50,0);
  }

  tft.setTextColor(HX8357_WHITE, HX8357_BLACK);
  // Uncomment next line to set the text colour to the last segment value!
  tft.setTextColor(colour, HX8357_BLACK);
  tft.setTextDatum(MC_DATUM);
  // Print value, if the meter is large then use big font 8, othewise use 4
  if (r > 84) {
    tft.setTextPadding(55*3); // Allow for 3 digits each 55 pixels wide
    tft.drawString(buf, x, y, 8); // Value in middle
  }
  else {
    tft.setTextPadding(3 * 14); // Allow for 3 digits each 14 pixels wide
    tft.drawString(buf, x, y, 4); // Value in middle
  }
  tft.setTextSize(1);
  tft.setTextPadding(0);
  // Print units, if the meter is large then use big font 4, othewise use 2
  tft.setTextColor(HX8357_WHITE, HX8357_BLACK);
  if (r > 84) tft.drawString(units, x, y + 60, 4); // Units display
  else tft.drawString(units, x, y + 15, 2); // Units display

  // Calculate and return right hand side x coordinate
  return x + r;
}

void drawAlert(int x, int y , int side, boolean draw)
{/*
  if (draw && !alert) {
    tft.fillTriangle(x, y, x+30, y+47, x-30, y+47, rainbow(95));
    tft.setTextColor(HX8357_BLACK);
    tft.drawCentreString("!", x, y + 6, 4);
    alert = 1;
  }
  else if (!draw) {
    tft.fillTriangle(x, y, x+30, y+47, x-30, y+47, HX8357_BLACK);
    alert = 0;
  }*/
}

// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;// Green is the middle 6 bits
  byte blue = 0; // Blue is the bottom 5 bits

  byte quadrant = value / 32;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}

// #########################################################################
// Return a value in range -1 to +1 for a given phase angle in degrees
// #########################################################################
float sineWave(int phase) {
  return sin(phase * 0.0174532925);
}

void printDigits(byte digits){
 // utility function for digital clock display: prints colon and leading 0
 tft.print(":");
 if(digits < 10)
   tft.print('0');
 tft.print(digits,DEC);  
}

