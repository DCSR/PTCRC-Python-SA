
/*  Version 200.05
 *. 
 * This should handle eight boxes with or without an inactive lever.
 * 
 * Two MCP23S17 port expanders are controled by the MCP23S17 library 
 * which was forked from from: github.com/MajenkoLibraries/MCP23S17
 * 
 * Chip0, Port0:  8 active levers - retract/extend
 * Chip0, Port1:  8 active lever LEDs 
 * Chip1, Port0:  8 active lever inputs
 * Chip1, Port1:  8  pumps
 * ************* if Two Levers *************************
 * Chip2, Port0:  8 inactive levers - retract/extend
 * Chip2, Port1:  8 inactive lever LEDs
 * Chip3, Port0:  8 active lever inputs
 * Chip3, Port1:  8 spare DIOs (alternate reinforcer?)
 * 
 */


#include <Queue.h> 
#include <SPI.h>        // Arduino Library SPI.h
#include "MCP23S17.h"   // Forked from Majenko MCP23S17 library

const uint8_t chipSelect = 10;  // All four chips use the same SPI chipSelect
MCP23S17 chip0(chipSelect, 0);  // Instantiate 16 pin Port Expander chip at address 0
MCP23S17 chip1(chipSelect, 1);  
MCP23S17 chip2(chipSelect, 2);    
MCP23S17 chip3(chipSelect, 3);   

#define On LOW
#define Off HIGH
#define Extend LOW
#define Retract HIGH

typedef struct tagTStamp {
   // tagTStamp is a structure identifier (or tag). It is not necessary 
   // to specify the tag except when you need a pointer to the struct 
   byte boxNum;
   char code;
   uint32_t mSecTime;
   byte state;
   byte index;
} TStamp;

// ********** variables **************************

Queue printQueue(sizeof(TStamp), 60, FIFO); // Instantiate printQueue, First In First Out
  // Note: maximum in the queue was 40 when quitting all eight boxes at once (w/o a delay)

const uint8_t ledPin = 5;
extern "C" char *sbrk(int i);   // used in FreeRam()
byte portOneValue, portTwoValue;
String instruction;

boolean lastLeverOneState[8] = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
boolean lastLeverTwoState[8] = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
volatile boolean tickFlag = false;

// program housekeeping variables
String inputString;
boolean sessionRunning = false;
boolean echoInput = false;
boolean twoLever = false;
unsigned long maxDelta = 0;
byte maxQueueRecs = 0;
boolean IOconnected = false;   // Set to false when debugging program without the breakout board connected

// ***************************  Box Class *************************************
class Box  {
  public:
    Box(int boxNum);
    void begin();
    void startSession();
    void endSession();
    void startBlock();
    void endBlock();
    void startTrial();
    void endTrial();
    void startIBI();
    void endIBI();
    void reinforce();
    void startTimeOut();
    void endTimeOut();
    void switchPump(int state);
    void switchStim1(int state);
    void switchStim2(int state);
    void moveLever1(int state);
    void moveLever2(int state);
    void tick();
    void handle_L1_Response(boolean bitState);
    void handle_L2_Response(boolean bitState);
    void setProtocolNum(int protocalNum);
    void setPumpDuration(int pumpDuration);
    void setParamNum(int paramNum);
    void setBlockDuration(int blockDuration);    
    void reportParameters();
    void getBlockTime();
    
    //************
  private:
    //these class members are only available within the class, 
    int _boxNum;
    int _tickCounts = 0; 
    boolean _verbose = false;
    boolean _holdDown = false;
    // defaults to a 6h FR1 session 
    int _protocolNum = 1;  // ['0: Do not run', '1: FR', '2: FR x 20', '3: FR x 40', 
                           //  '4: PR', '5: TH', '6: IntA: 5-25', '7: HD',  '8: IntA-HD']
    int _paramNum = 1;
    int _PRstepNum = 1;
    int _boxPhase = 0;     // 0 = preStart; 1 = timeIn; 2 = timeOut; 3 = IBI; 4 = finished
    unsigned long _blockDuration = 21600;  // 60 * 60 * 6 = 21600 seconds = 6 hrs;
    unsigned long _IBILength = 0;
    unsigned int _maxBlockNumber = 1;
    unsigned int _maxTrialNumber = 999;
    unsigned int _responseCriterion = 1;  // default to FR1
    int _pumpDuration = 400;    // default to 4 seconds - 400 x 10 mSec ticks = 4000 mSec
    int _THPumpTimeArray[12] = {316, 200, 126, 79, 50, 32, 20, 13, 8, 5, 3, 2};    // different from SelfAdmin201.ino
    int _stimDuration = 400;   // default to same as pumpDuration;
    int _pumpTimer = 0;
    int _stimTimer = 0;
    unsigned long _blockTime = 0;    // unsigned int would only allow 65,535 seconds = 18.2 hours     
    unsigned long _startTime = 0;
    unsigned int _blockNumber = 0;
    unsigned int _trialNumber = 0;
    unsigned int _trialResponses = 0;     
};

// **************  Box Class Procedures *************************************
Box::Box(int boxNum) {    // This is the constructor
  _boxNum = boxNum; 
}

void Box::begin() {       // Not sure this procedure is necessary
  _protocolNum = 1;
}

void Box::startSession() { 
  // ******** set Protocol defaults here ********
  // Python protocol list as defined in SA200.py:
  // ['0: Do not run', '1: FR', '2: FR x 20', '3: FR x 40',
  //  '4: PR', '5: TH', '6: IntA: 5-25', '7: HD', '8: IntA-HD']

  if (_protocolNum == 0) _boxPhase = 5;  // if "do not run" then boxPhase = "finished"
  else  {
      _responseCriterion = _paramNum;
      if (_protocolNum == 1)      _maxTrialNumber = 999;     
      else if (_protocolNum == 2) _maxTrialNumber = 20;   
      else if (_protocolNum == 3) _maxTrialNumber = 40;
      else if (_protocolNum == 4) _PRstepNum  = _paramNum;  // PR  
      else if (_protocolNum == 5) {      // TH
          _blockDuration = 600;          // 600 seconds = 10 min
          _IBILength = 0;                // no IBI
          _maxBlockNumber = 12;          // 12 blocks
      }
      else if (_protocolNum == 6) {      // IntA 5-25 6h
          _blockDuration = 300;          // 300 seconds = 5 min
          _IBILength = 1500;             // 1500 seconds = 25 min
          _maxBlockNumber = 12;          // 12 blocks
      }
      else if (_protocolNum == 7) {      // HD
          _holdDown = true;
          _blockDuration = 10800;        // 60 * 60 * 3 = 10800 seconds = 3 hrs; 
      }
      else if (_protocolNum == 8) {      // IntA-HD 5-25
          _blockDuration = 300;          // 300 seconds = 5 min
          _IBILength = 1500;             // 1500 seconds = 25 min
          _maxBlockNumber = 12;          // 12 blocks
      }
      _startTime = millis();
      _blockNumber = 0;  
      _pumpTimer = 0; 
      _stimTimer = 0;
      // sessionRunning = true;     
      TStamp tStamp = {_boxNum, 'G', millis() - _startTime, 0, 9};
      printQueue.push(&tStamp);
      startBlock();
      if (twoLever) moveLever2(Extend);
  }
}

void Box::endSession () {
    moveLever1(Retract);
    moveLever2(Retract);
    switchStim1(Off);
    switchPump(LOW);
    _pumpTimer = 0;
    _stimTimer = 0; 
    _boxPhase = 4;    
    TStamp tStamp = {_boxNum, 'E', millis() - _startTime, 0, 9};
    printQueue.push(&tStamp);
}

void Box::startBlock() {
  _blockTime = 0;
  _trialNumber = 0;
  _blockNumber++;
  TStamp tStamp = {_boxNum, 'B', millis() - _startTime, 0, 9};
  printQueue.push(&tStamp);  
  if (_protocolNum == 5){                                    // TH
      _pumpDuration = _THPumpTimeArray[_blockNumber - 1];    // zero indexed array; block 1 = index 0
  }
  startTrial();
}

void Box::endBlock() {    
     TStamp tStamp = {_boxNum, 'b', millis() - _startTime, 0, 9};
     printQueue.push(&tStamp);
     if (_blockNumber < _maxBlockNumber) startIBI();
     else endSession();
}

void Box::startTrial() {   
    _trialResponses = 0;
    _trialNumber++;
    if (_protocolNum == 4) _responseCriterion = round((5 * exp(0.2 * _PRstepNum)) - 5);    // Sched = PR 
    _PRstepNum++;
    moveLever1(Extend);             // extend lever
    _boxPhase = 1;                // 0 = preStart; 1 = timeIn; 2 = timeOut; 3 = IBI; 4 = finished  
    TStamp tStamp = {_boxNum, 'T', millis() - _startTime, 0, 9};
    printQueue.push(&tStamp);
}

void Box::endTrial() { 
  if (_protocolNum != 5) moveLever1(Retract);       // if not TH then retract lever
}

void Box::startIBI() {   
    if (_IBILength == 0) startBlock();
    else
    {   moveLever1(Retract);
        _blockTime = 0;    // tick will handle when to end IBI 
        _boxPhase = 3;     // IBI
    }    
}

void Box::endIBI() {
     startBlock();
}

void Box::reinforce() {
    switchPump(HIGH);            // On
    _pumpTimer = _pumpDuration;  // 400 (4 sec) or adjusted for body weight
}

void Box::startTimeOut() {   
    switchStim1(On); 
    if (_protocolNum == 2) _stimTimer = _stimDuration;
    else _stimTimer = _pumpDuration;
    _boxPhase = 2;                  //timeOut    
    TStamp tStamp = {_boxNum, 't', millis() - _startTime, 0, 9};
    printQueue.push(&tStamp);
}

void Box::endTimeOut() {   
    switchStim1(Off);
    if (_trialNumber < _maxTrialNumber) startTrial(); 
    else endBlock();
}

void Box::switchPump(int state) {
    // boxNum 0..7 maps to pin 0..7 on chip1
    if (IOconnected) chip1.digitalWrite(_boxNum+8,state);             
    // HIGH = On
    if (state) {
          TStamp tStamp = {_boxNum, 'P', millis() - _startTime, 1, 2};
          printQueue.push(&tStamp);
    }
    else {
          TStamp tStamp = {_boxNum, 'p', millis() - _startTime, 0, 2};
          printQueue.push(&tStamp);
    }
    // The Pump CheckBox is index 2 
}

void Box::switchStim1(int state) {   
    if (IOconnected) chip0.digitalWrite(_boxNum+8,state);   // boxNum 0..7 maps to pin 8..15 on chip0
    // HIGH = OFF
    if (state) {
          TStamp tStamp = {_boxNum, 's', millis() - _startTime, 0, 3};
          printQueue.push(&tStamp);
    }
    else {
          TStamp tStamp = {_boxNum, 'S', millis() - _startTime, 1, 3};
          printQueue.push(&tStamp);
    }
    // StimCheckBox is index 3     
}

void Box::switchStim2(int state) {   
    if (IOconnected) chip2.digitalWrite(_boxNum+8,state);   // boxNum 0..7  maps to pin 8..15 on chip2
    // HIGH = OFF
    if (state) {
          TStamp tStamp = {_boxNum, 'c', millis() - _startTime, 0, 4};
          printQueue.push(&tStamp);
    }
    else {
          TStamp tStamp = {_boxNum, 'C', millis() - _startTime, 1, 4};
          printQueue.push(&tStamp);
    }
    // StimCheckBox is index 4     
}

void Box::moveLever1(int state) {          // boxNum 0..7  maps to pin 0..7 on chip0
    if (IOconnected) chip0.digitalWrite(_boxNum,state);
    // HIGH = Retract; LOW = extend
    if (state) {
           TStamp tStamp = {_boxNum, '-', millis() - _startTime, 0, 0};
           printQueue.push(&tStamp); 
    }
    else {
          TStamp tStamp = {_boxNum, '=', millis() - _startTime, 1, 0};
          printQueue.push(&tStamp);
    }
    // Lever1 (active) CheckBox is index 0 
}

void Box::moveLever2(int state) {          // boxNum 0..7  maps to pin 0..7 on chip2
    if (IOconnected) chip2.digitalWrite(_boxNum,state);
    // HIGH = Retract
    if (state) { 
          TStamp tStamp = {_boxNum, ',', millis() - _startTime, 0, 1};
          printQueue.push(&tStamp);
    }
    else {
         TStamp tStamp = {_boxNum, '-', millis() - _startTime, 1, 1};
         printQueue.push(&tStamp);
    }
    // Lever2 (inactive) CheckBox is index 1 
}

void Box::tick() {                           // do stuff every 10 mSec
    if (_pumpTimer > 0) {  
      _pumpTimer--;
      if (_pumpTimer == 0) switchPump(LOW);  // Off
    }
    if (_stimTimer > 0)  {  
      _stimTimer--; 
      if (_stimTimer == 0) endTimeOut();  
    }   
    _tickCounts++;
    if (_tickCounts == 100)    {         // every second
       _tickCounts = 0;        
       if (_boxPhase > 0 && _boxPhase < 4)   {   // boxPhase 1,2 or 3: started and not finished 
           _blockTime++; 
           TStamp tStamp = {_boxNum, '*', _blockTime, 0, 9};
           printQueue.push(&tStamp);                 
           if (_boxPhase == 1 || _boxPhase == 2)  {         // timeIn or TimeOut in a block          
               if (_blockTime >= _blockDuration) endBlock(); 
           }    
           else {
               if (_blockTime >= _IBILength) endIBI();     
           }
        } 
    }
}   

void Box::handle_L1_Response(boolean bitState) {
   /*  Sends something like "0 L 23456 0 9" 
   *  i.e boxNum, code, mSecTime, state and index
   *  Note: state and index are irrelevant, only used to update checkboxes
   */    
    if (bitState == 0){
         TStamp tStamp = {_boxNum, 'L', millis() - _startTime, 0, 9};
         printQueue.push(&tStamp);
         if (_boxPhase == 1) {             // timeIn
               if (_holdDown == false) {   // normal stuff
                    _trialResponses++;
                    if (_trialResponses >= _responseCriterion) { 
                        /*if (_protocolNum == 5 && _blockNumber == 12) {
                        // do nothing. This is because the 12 block of TH has pumptime = 0
                        // check on this. The pumptime is now 2 in block 12   */
                        reinforce();
                        endTrial();
                        startTimeOut();               
                    }
               }
               else {                       // (_holdDown == true)
                  switchPump(HIGH);         // Switch On
               }
          }
    }
    else {                  // bitState == 1
         TStamp tStamp = {_boxNum, 'l', millis() - _startTime, 0, 9};
         printQueue.push(&tStamp);
         // if (_boxPhase == 1 && _holdDown == true) {     // HD
            switchPump(LOW);                // Switch Off
    }
}

void Box::handle_L2_Response(boolean bitState) {           // Inactive lever press
    if (bitState == 0){
       TStamp tStamp = {_boxNum, 'J', millis() - _startTime, 0, 9};
       printQueue.push(&tStamp);
    }
    /*  Probably no need to report lever up on inactive lever
    else {
       TStamp tStamp = {_boxNum, 'j', millis() - _startTime, 0, 9};
       printQueue.push(&tStamp);
    } */
}

void Box::setProtocolNum(int protocolNum) {
  _protocolNum = protocolNum;
}

void Box::setPumpDuration(int pumpDuration) {
  _pumpDuration = pumpDuration;
}

void Box::setParamNum(int paramNum) {
  _paramNum = paramNum;
}

void Box::setBlockDuration(int blockDuration) {
  _blockDuration = blockDuration;
}

void Box::reportParameters() { 
  Serial.print("9 "+String(_boxNum)+":"+String(_protocolNum)+":");
  Serial.print(String(_responseCriterion)+":"+String(_blockDuration)+":");
  Serial.print(String(_pumpDuration)+":"+String(_stimDuration));
  Serial.println("-"+String(_maxTrialNumber));
} 

void Box::getBlockTime() {
  Serial.println("9 "+String(_boxNum)+"_BTime="+ String(_blockTime));
  // TStamp tStamp = {_boxNum, '9', millis() - _startTime, 0, 9};
  // printQueue.push(&tStamp);
}

// ************** End of Box Class ******************************************

// *************  instantiate eight boxes and boxArray  *********************
Box box0(0);  
Box box1(1); 
Box box2(2); 
Box box3(3); 
Box box4(4);  
Box box5(5); 
Box box6(6); 
Box box7(7); 
Box boxArray[8] = {box0, box1, box2, box3, box4, box5, box6, box7}; 

// *************************** Timer stuff **********************************
void init_10_mSec_Timer() { 
  REG_GCLK_GENDIV = GCLK_GENDIV_DIV(3) |          // Divide the 48MHz clock source by divisor 3: 48MHz/3=16MHz
                    GCLK_GENDIV_ID(4);            // Select Generic Clock (GCLK) 4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |           // Set the duty cycle to 50/50 HIGH/LOW
                     GCLK_GENCTRL_GENEN |         // Enable GCLK4
                     GCLK_GENCTRL_SRC_DFLL48M |   // Set the 48MHz clock source which is now 16mHz (I think)
                     GCLK_GENCTRL_ID(4);          // Select GCLK4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization
  // Feed GCLK4 to TC4 and TC5
  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |         // Enable GCLK4 to TC4 and TC5
                     GCLK_CLKCTRL_GEN_GCLK4 |     // Select GCLK4
                     GCLK_CLKCTRL_ID_TC4_TC5;     // Feed the GCLK4 to TC4 and TC5
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization
  REG_TC4_COUNT16_CC0 = 0x270;                    // 0x270 = 624; Interval = 1/(160000000 / 256) * 625 = 0.01 Sec or every 10 mSec
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);       // Wait for synchronization
  //NVIC_DisableIRQ(TC4_IRQn);
  //NVIC_ClearPendingIRQ(TC4_IRQn);
  NVIC_SetPriority(TC4_IRQn, 0);    // Set the Nested Vector Interrupt Controller (NVIC) priority for TC4 to 0 (highest)
  NVIC_EnableIRQ(TC4_IRQn);         // Connect TC4 to Nested Vector Interrupt Controller (NVIC)

  REG_TC4_INTFLAG |= TC_INTFLAG_OVF;              // Clear the interrupt flags
  REG_TC4_INTENSET = TC_INTENSET_OVF;             // Enable TC4 interrupts
  // REG_TC4_INTENCLR = TC_INTENCLR_OVF;          // Disable TC4 interrupts
 
  REG_TC4_CTRLA |= TC_CTRLA_PRESCALER_DIV256 |    // Set prescaler to 256, 16MHz/256 = 62500 Hz
                   TC_CTRLA_WAVEGEN_MFRQ |        // Put the timer TC4 into match frequency (MFRQ) mode 
                   TC_CTRLA_ENABLE;               // Enable TC4
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);       // Wait for synchronization 
}

void TC4_Handler()                                // Interrupt Service Routine (ISR) for timer TC4
{     
  // Check for overflow (OVF) interrupt
  if (TC4->COUNT16.INTFLAG.bit.OVF && TC4->COUNT16.INTENSET.bit.OVF)  {
    tickFlag = true;
    REG_TC4_INTFLAG = TC_INTFLAG_OVF;             // Clear the OVF interrupt flag
  }
}
// **************************  End Timer stuff ****************************

void turnStuffOff(){
  if (IOconnected) {
     chip0.writePort(0xFFFF);
     chip1.writePort(1,0x00);    
     chip2.writePort(0xFFFF);
     chip3.writePort(1,0x00); 
  }      
}

void setup() {
  Serial.begin(115200);
  delay(1000); 
  Serial.println("9 SA200.ino");
  pinMode(ledPin, OUTPUT);   // GPIO 5
  if (IOconnected == true) {
      chip0.begin();     // This starts SPI and set the chip select
      chip1.begin();     // pin (10) to OUTPUT
      chip2.begin();                             // different from SelfAdmin201.ino
      chip3.begin();                             // different from SelfAdmin201.ino
      for (uint8_t i = 0; i <= 15; i++) {
        chip0.pinMode(i,OUTPUT);             // Set chip0 to OUTPUT
        chip2.pinMode(i,OUTPUT);             // Set chip2 to OUTPUT
      }
      for (uint8_t i = 0; i <= 7; i++)  {
        chip1.pinMode(i,INPUT_PULLUP);          // Ver 200.04
        chip3.pinMode(i,INPUT_PULLUP);          // Ver 200.04
      }
      for (uint8_t i = 8; i <= 15; i++) {
        chip1.pinMode(i, OUTPUT);               // Ver 200.04
        chip3.pinMode(i, OUTPUT);               // Ver 200.04
      }
      turnStuffOff();  
      portOneValue = chip1.readPort(0);          
      portTwoValue = chip3.readPort(0);          
      // Serial.println(portOneValue,BIN);
  }
  boxArray[0].begin();
  boxArray[1].begin();
  boxArray[2].begin();
  boxArray[3].begin();
  boxArray[4].begin();
  boxArray[5].begin();
  boxArray[6].begin();
  boxArray[7].begin();
  init_10_mSec_Timer();    
}

void checkLeverOne() {
   static byte oldPortOneValue = 255;
   boolean bitState;       
   portOneValue = chip1.readPort(0);
   if(portOneValue != oldPortOneValue) {
         oldPortOneValue = portOneValue;
         // Serial.println (portOneValue,BIN);
         for (byte i = 0; i < 8; i++) {
             bitState = bitRead(portOneValue,i);
             if (bitState != lastLeverOneState[i]) {     
                  lastLeverOneState[i] = bitState;
                  boxArray[i].handle_L1_Response(bitState);
             }
         }
    }    
}           


void checkLeverTwo() {
   static byte oldPortTwoValue = 255;
   boolean bitState;
   portTwoValue = chip3.readPort(0);
   if(portTwoValue != oldPortTwoValue) {
        oldPortTwoValue = portTwoValue;
         // Serial.println (portTwoValue,BIN);
         for (byte i = 0; i < 8; i++) {
             bitState = bitRead(portTwoValue,i);
             if (bitState != lastLeverTwoState[i]) {
                  lastLeverTwoState[i] = bitState;
                  boxArray[i].handle_L2_Response(bitState);
             }
         }    
    }           
}

/*
void createTimeStamps(int num) {
   int i;
   for (i = 0 ; i < num ; i++) {
      TStamp tStamp = {i, '*', millis(), 0, 2};
      printQueue.push(&tStamp);
      Serial.print("loading timestamp ");
      Serial.print(printQueue.nbRecs());   
      delay(10); 
      Serial.println("9 freeRam="+String(freeRam()));
  }
}
*/

void getInputString() {
    while (Serial.available() > 0) {               // repeat while something in the buffer
      char aChar = Serial.read();                  // get the next character
      if (aChar == '>') handleInputString();       // end of instruction string
      else if (aChar == '<') inputString = "";     // beginning of instruction
      else inputString += aChar;                   // add the character to inputString
    }
}

void handleInputString()
{  byte firstSpaceIndex;
   byte secondSpaceIndex;
   String stringCode = "";
   String num1Code = "";
   String num2Code = "";
   int num1 = 0;
   int num2 = 0;
   if (inputString.length() > 0) {                         // makes sure that there is something in inputString 
     firstSpaceIndex = inputString.indexOf(' ');           // finds out if the string contains a space
     if (firstSpaceIndex == -1) stringCode = inputString;  // if it doesn't, num1Code and num2Code aren't used; num1 and num2 remain 0 
     else {                                                // if there is at least one space
        stringCode = inputString.substring(0,firstSpaceIndex);            // stringCode is everything before the space
        secondSpaceIndex = inputString.indexOf(' ',firstSpaceIndex+1);    // determine if there is a seocnd space
        if (secondSpaceIndex == -1) {
             num1Code = inputString.substring(firstSpaceIndex+1,inputString.length());  // num1Code is everything after the space
        }
        else {num1Code = inputString.substring(firstSpaceIndex+1,secondSpaceIndex);     // everything from first space to second space
              num2Code = inputString.substring(secondSpaceIndex+1,inputString.length());// everything
              num2 = num2Code.toInt();
        }
        num1 = num1Code.toInt();         // turn the text value into an interger
     }
     if (echoInput) Serial.println("9 <"+stringCode+":"+num1+":"+num2+">"); 
     if (stringCode == "G")     boxArray[num1].startSession();
     else if (stringCode == "Q")     boxArray[num1].endSession(); 
     else if (stringCode == "P")     boxArray[num1].switchPump(HIGH);
     else if (stringCode == "p")     boxArray[num1].switchPump(LOW);
     else if (stringCode == "SCHED") boxArray[num1].setProtocolNum(num2);
     else if (stringCode == "PUMP")  boxArray[num1].setPumpDuration(num2); 
     else if (stringCode == "RATIO") boxArray[num1].setParamNum(num2);
     else if (stringCode == "TIME")  boxArray[num1].setBlockDuration(num2);
     else if (stringCode == "R")     boxArray[num1].reportParameters();
     else if (stringCode == "=")     boxArray[num1].moveLever1(Extend);   // extend lever1
     else if (stringCode == "-")     boxArray[num1].moveLever1(Retract);    // retract lever1
     else if (stringCode == "~")     boxArray[num1].moveLever2(Extend);   // extend lever2  
     else if (stringCode == ",")     boxArray[num1].moveLever2(Retract);    // retract lever2
     else if (stringCode == "s")     boxArray[num1].switchStim1(Off);
     else if (stringCode == "S")     boxArray[num1].switchStim1(On);
     else if (stringCode == "c")     boxArray[num1].switchStim2(Off);
     else if (stringCode == "C")     boxArray[num1].switchStim2(On);
     else if (stringCode == "T")     twoLever = true;
     else if (stringCode == "t")     twoLever = false;
     // debug stuff 
     else if (stringCode == "L1")    boxArray[num1].handle_L1_Response(0);  
     // else if (stringCode == "chip0")      chip0.digitalWrite(num1,num2); 
     else if (stringCode == "off")   turnStuffOff();
     else if (stringCode == "i")     timeUSB();
     else if (stringCode == "E")     echoInput = !echoInput;
     else if (stringCode == "M")     reportMaxDelta();
     else if (stringCode == "D")     reportDiagnostics(); 
     else if (stringCode == "B")     boxArray[num1].getBlockTime();
     else if (stringCode == "V")     {
        Serial.println("9 v200.05");
        if (IOconnected == false) {
           Serial.println("9 IOconnected=false");
           Serial.println("9 --------WARNING------"); 
        }
     }
     inputString = "";
   }
}

int freeRam () {
  char stack_dummy = 0;
  return &stack_dummy - sbrk(0);
}

void reportMaxDelta() {
    Serial.println("9 maxDelta="+String(maxDelta));
    maxDelta = 0;
}

void reportDiagnostics() {
   reportMaxDelta();
   Serial.println("9 maxQueueRecs="+String(maxQueueRecs));
   Serial.println("9 freeRam="+String(freeRam()));   
}

void timeUSB() {
   unsigned long delta, micro1;
   micro1 = micros();
   for (uint8_t i = 0; i < 30; i++)  {
     Serial.println("9 timeUSB i ="+String(i));  
  }
   delta = micros() - micro1;
   Serial.println("9 timeUSB="+String(delta));  
}

void sendOneTimeStamp() {
   if (printQueue.isEmpty()==false) {
      if (printQueue.nbRecs() > maxQueueRecs) maxQueueRecs = printQueue.nbRecs();
      TStamp tStamp;
      printQueue.pull(&tStamp);
      if (tStamp.index == 9) {
           Serial.println(String(tStamp.boxNum)+" "+tStamp.code+" "+String(tStamp.mSecTime)) ;      
      }
      else {
           Serial.println(String(tStamp.boxNum)+" "+tStamp.code+" "+String(tStamp.mSecTime)
           +" "+String(tStamp.state)+" "+String(tStamp.index));
      }
   }
}

void tick()    {  
   static int tickCounts = 0;  // used for flashing LED and printTime
   int boxIndex;
   unsigned long delta, micro1;
   micro1 = micros();
   tickCounts++;
   for (uint8_t i = 0; i < 8; i++)  {
     boxArray[i].tick();
   }
   getInputString();
   if (IOconnected) {
       checkLeverOne();
       if (twoLever) checkLeverTwo();
   }
   sendOneTimeStamp();
   delta = micros() - micro1;
   if (delta > maxDelta) maxDelta = delta;   
   if (tickCounts == 100) {
       digitalWrite(ledPin, !digitalRead(ledPin));
       tickCounts = 0;
   }
}

void loop() {
   if (tickFlag) {
      tickFlag = false;
      tick();
   }
}


