/*    
 * This should handle eight boxes with or without an inactive lever.
 * 
 * Two MCP23S17 port expanders are controlLed by the MCP23S17 library 
 * which was forked from from: github.com/MajenkoLibraries/MCP23S17
 * 
 * Chip0, Port0:  8 L1 levers - retract/extend
 * Chip0, Port1:  8 L1 lever LEDs 
 * Chip1, Port0:  8 L1 inputs
 * Chip1, Port1:  8 Pumps
 * ************* if Two Levers *************************
 * Chip2, Port0:  8 L2 levers - retract/extend
 * Chip2, Port1:  8 L2 lever LEDs 
 * Chip3, Port0:  8 L2 inputs
 * Chip3, Port1:  8 AUX output (perhaps food hoppers)
 * 
 */


#include <cppQueue.h>
#include <SPI.h>        // Arduino Library SPI.h
#include "MCP23S17.h"   

String verStr = "Ver301.01";
const uint8_t chipSelect = 10;  // All four chips use the same SPI chipSelect
MCP23S17 chip0(chipSelect, 0);  // Instantiate 16 pin Port Expander chip at address 0
MCP23S17 chip1(chipSelect, 1);  
MCP23S17 chip2(chipSelect, 2);    
MCP23S17 chip3(chipSelect, 3); 

// SYSVARS
boolean checkLever1 = true;
boolean checkLever2 = true;
boolean checkOutputs = false;
boolean Verbose = true;   // note that "verbose" (small v) is a reserved word
boolean showMaxDelta = false;
boolean abortEnabled = false;
boolean showDebugOutput = false;
boolean sessionAborted = false; 

byte portOneValue = 255, portTwoValue = 255;

// Variables needed for error detection - see HD_DEMO.ino
// The following variables are used to track the state of each output port
// Output Ports Values
byte L1_Position = 0xFF;
byte L1_LED_State = 0xFF;      
byte pumpState = 0x00;          // bitwise OR of (pumpStateL1 | pumpStateL2);
byte pumpStateL1 = 0x00;        // Determined by lever 1
byte pumpStateL2 = 0x00;        // Determined by lever 2 (HD)
byte L2_Position = 0xFF;        // Retracted
byte L2_LED_State = 0xFF;       // Off

#define On true
#define Off false
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

byte boxesRunning = 0;
const uint8_t ledPin = 5;
extern "C" char *sbrk(int i);   // used in FreeRam()
String instruction;

boolean lastLeverOneState[8] = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
boolean newLeverOneState[8] = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
boolean newResponse[8] = {false,false,false,false,false,false,false,false};
byte ticks[8] = {0,0,0,0,0,0,0,0};
boolean lastLeverTwoState[8] = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
boolean newLeverTwoState[8] = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
boolean leverTwoExists = false;

volatile boolean tickFlag = false;

// program housekeeping variables
String inputString;
boolean echoInput = false;
unsigned long maxDelta = 0;
byte maxQueueRecs = 0;

// enum  states { PRESTART, BLOCK, IBI, L2_HD, FINISHED };
enum states { PRESTART, L1_ACTIVE, L1_TIMEOUT, IBI, L2_HD, FINISHED };

// ******************** Box Class **************

class Box  {
  public:
    Box (int boxNum) {                          // constructor
        _boxNum = boxNum;
    }
    void startSession();
    void setProtocolDefaults();
    void endSession();
    void tick();
    void handle_L1_Response();
    void handle_L2_Response(byte state);
    void setProtocolNum(int protocalNum);
    void deliverFoodPellet();                   // stub
    void switchTimedPump(int state); 
    
    // void switchRewardPortOn(boolean timed);
    // void switchRewardPortOff();
    
    void moveLeverOne(int state);    
    void moveLeverTwo(int state);    
    void switchStim1(boolean state);
    void switchStim2(boolean state);
    void setPumpDuration(int pumpDuration);
    void setParamNum(int paramNum);
    void setBlockDuration(int blockDuration); 
    void setIBIDuration(int IBIDuration);
    void reportParameters();
    void getBlockTime();
    
    boolean pumpOn = false;

    unsigned long _startTime = 0;   // Used for lever timestamnps 
    
    //************
        
  private:
    int _boxNum;
    int _protocolNum = 1;  // ['0: Do not run', '1: FR', '2: FR x 20', '3: FR x 40', '4: PR', '5: TH', 
                           // '6: IntA: 5-25', '7: Flush', '8: L2-HD', '9: IntA-HD', '10: 2L-PR-HD']
    int _paramNum = 1;
    int _pumpOnTime;          // set in StartSession()
    int _pumpOffTime = 20;    // default
    int _pumpOnTicker = 0;
    int _pumpOffTicker = 0; 
    int _tickCounts = 0;
    void startBlock();
    void endBlock();
    void startTrial();
    void startHDTrial();
    void endTrial();
    void endHDTrial();
    void startIBI();
    void endIBI();
    void reinforce();
    void startTimeOut();
    void endTimeOut(); 
    
    // boolean _timeOut = false; 
    boolean _timedPumpOn = false;                    
    boolean _schedPR = false;
    boolean _schedTH = false; 
    int _PRstepNum = 1;   
    states _boxState = PRESTART;
    // Block
    boolean _startOnLeverOne = true;
    unsigned long _blockDuration = 21600;  // default to 6 hrs = 60 * 60 * 6 = 21600 seconds 
    int _blockDurationInit = 21600;        // default to 6 hrs
    unsigned long _blockTime = 0;    // unsigned int would only allow 65,535 seconds = 18.2 hours
    unsigned int _blockNumber = 0;
    unsigned int _maxBlockNumber = 1;  
    // Trial 
    boolean _2L_PR = false;
    unsigned int _responseCriterion = 1;  // default to FR1
    unsigned int _trialNumber = 0;
    unsigned int _maxTrialNumber = 999;
    unsigned int _trialResponses = 0;
    // Reinforce 
    boolean _unitDose = true;     // used to direct 2L-PR_HD to startHDTrial
    int _pumpDuration = 400;    // 400 x 10 mSec = 4,000 mSec = 4 seconds;
    int _pumpTime = 0;    
    int _THPumpTimeArray[13] = {316, 316, 200, 126, 79, 50, 32, 20, 13, 8, 5, 3, 2};
    // int _THPumpTimeArray[13] = {100, 100, 50, 40, 30, 20, 10, 9, 8, 7, 5, 3, 2};    
    int _timeOutTime = 0;
    int _timeOutDuration = 400;  // default to 4 sec 
    // HD
    unsigned long _HDTime = 0;    
    unsigned long _HD_Duration = 20;  // default to 20 sec - used with 2L=PR-HD
    // IBI     
    unsigned long _IBIDuration = 0;
    int _IBIDurationInit = 0;  // default to no IBI
    unsigned long _IBITime = 0;
};


// ********** Box Procedures *************

void Box::startBlock() {
  TStamp tStamp = {_boxNum, 'B', millis() - _startTime, 0, 9};
  printQueue.push(&tStamp);
  _blockTime = 0;
  _trialNumber = 0;
  _blockNumber++;  
  if (_schedTH == true){                                       // TH
      _pumpDuration = _THPumpTimeArray[_blockNumber - 1];    // zero indexed array; block 1 = index 0
      _timeOutDuration = _pumpDuration;
      if (_blockNumber == 1) {
        _blockDuration = 21600;          // 60 * 60 * 6 = 21600 seconds = 6 hrs;
        _maxTrialNumber = 4;
      }
      else {
        _blockDuration = _blockDurationInit;              // Block duration in seconds; 
        _maxTrialNumber = 999;   
      }  
  }
  if (_protocolNum != 7) {                                // If not Flush
    if (_startOnLeverOne) startTrial();
    else startHDTrial();    
  }
}

void Box::endBlock() {  
   TStamp tStamp = {_boxNum, 'b', millis() - _startTime, 0, 9};
   printQueue.push(&tStamp);
   if (_protocolNum == 7) {             // Flush
      switchTimedPump(On);
   }   
   else if (_blockNumber == _maxBlockNumber) endSession();
   else {
     if (_IBIDuration > 0) startIBI();
     else startBlock();
   }
}

void Box::startTrial() { 
   TStamp tStamp = {_boxNum, 'T', millis() - _startTime, 0, 9};
   printQueue.push(&tStamp);
   _trialResponses = 0;
   _trialNumber++;
   if (_schedPR == true) {
      _responseCriterion = round((5 * exp(0.2 * _PRstepNum)) - 5);    // Sched = PR
      _PRstepNum++;
   }
   moveLeverOne(Extend);     // extend lever
   _boxState = L1_ACTIVE;    
}

void Box::startHDTrial() {
   TStamp tStamp = {_boxNum, 'Z', millis() - _startTime, 0, 9};
   printQueue.push(&tStamp);
   moveLeverTwo(Extend);     // extend lever
   _boxState = L2_HD;
   _HDTime = 0;
}

void Box::endTrial() {
   TStamp tStamp = {_boxNum, 't', millis() - _startTime, 0, 9};
   printQueue.push(&tStamp);
   if (_protocolNum != 5) moveLeverOne(Retract);       // if not TH then retract lever
}

void Box::endHDTrial() {
   TStamp tStamp = {_boxNum, 'z', millis() - _startTime, 0, 9};
   printQueue.push(&tStamp);
   switchTimedPump(Off);
   moveLeverTwo(Retract);
   if (_2L_PR) startTrial();
   else endBlock(); 
}

void Box::startIBI() { 
   TStamp tStamp = {_boxNum, 'I', millis() - _startTime, 0, 9}; 
   printQueue.push(&tStamp); 
   moveLeverOne(Retract);
   _IBITime = 0;         // tick will handle when to end IBI 
   _boxState = IBI;          
}

void Box::endIBI() {
   TStamp tStamp = {_boxNum, 'i', millis() - _startTime, 0, 9}; 
   printQueue.push(&tStamp);
   startBlock();
}

void Box::reinforce() { 
    if (_unitDose) {
      switchTimedPump(On);
      startTimeOut();
    }
    else startHDTrial();
}

void Box::startTimeOut() {
    TStamp tStamp = {_boxNum, 'O', millis() - _startTime, 0, 9}; 
    printQueue.push(&tStamp);
    switchStim1(On); 
    _timeOutTime = 0;       // _timeOutTime counts up _timeOutDuration
    _boxState = L1_TIMEOUT;
    // _timeOut = true;         
}

void Box::endTimeOut() {
    TStamp tStamp = {_boxNum, 'o', millis() - _startTime, 0, 9}; 
    printQueue.push(&tStamp); 
    switchStim1(Off);
    if (_trialNumber < _maxTrialNumber) startTrial(); 
    else {
      endBlock();
    }  
}

void Box::deliverFoodPellet() {
   /* This is a stub.
      Need to define timing varaibles for feeder: feederTime, feederDuration etc
      chip3, pins 8..15 map to boxNum 0..7 to AUX output   
      Need to define gobal variable - feederState and mimic how it is done
      in Box::switchStim1(boolean state)
      feederState = 0xFF   <- OFF;  Feeders discharge when supplied with GND
      Note that drug vs food uses different ports and 5VDC vs GND to switch ON
      Future use will likely depend on a protocol and local variables 
      determining whether to call switchTimedPumpOn() or deliverFoodPellet() 
      timestamp Hopper (H,h)
   */  
}

void Box::switchTimedPump(int state) { 
    // Pumps for boxNum 0..7 maps to pins 8..15 map on chip1
    // Pump CheckBox is index 2
    // pumpState = 0x00    <- OFF
    // Pump off with bitClear()
    // Pump On with bitSet() 
    // Old way: chip1.digitalWrite(_boxNum+8,level);
    
    if (state == On) {
          _pumpTime = 0;
          _timedPumpOn = true;
          bitSet(pumpStateL1,_boxNum);        // Pump is on when bit = 1 
          TStamp tStamp = {_boxNum, 'P', millis() - _startTime, 1, 2};
          printQueue.push(&tStamp);
    }
    else {                                    // OFF
          _timedPumpOn = false;
          bitClear(pumpStateL1,_boxNum);      // Pump is off when bit = 0
          TStamp tStamp = {_boxNum, 'p', millis() - _startTime, 0, 2};
          printQueue.push(&tStamp);
    }
    // chip1.writePort(1,pumpStateL1State);  <- called in Tick() 
}

void Box::moveLeverOne(int state) { 
   /*  chip0, pins 0..7 map to boxNum 0..7 to extend/retract Lever 1
    *  Lever1 (active) CheckBox is index 0
    *  Previous version: chip0.digitalWrite(_boxNum,state);
    *  Revision: set bit in L1_Position then chip0.writePort(0,L1_Position);
    */
   
   if (state == Extend) {                 // Defined LOW = Extend = 0 
      bitClear(L1_Position,_boxNum);      // Set bit to 0
      TStamp tStamp = {_boxNum, '.', millis() - _startTime, 1, 0};  // set checkbox with 1
      printQueue.push(&tStamp); 
   }
   else {                                 // Defined LOW = Extend = 0
      bitSet(L1_Position,_boxNum);        // Set bit to 1
      TStamp tStamp = {_boxNum, '=', millis() - _startTime, 0, 0};  // uncheck checkbox with 0
      printQueue.push(&tStamp);
   }
   chip0.writePort(0,L1_Position); 
}

void Box::moveLeverTwo(int state) {
   /*   chip2, pins 0..7 map to boxNum 0..7 to extend/retract Lever 2
    *   Lever2 (HD) CheckBox is index 1
    *   Previous version: chip2.digitalWrite(_boxNum,state);
    *   Revision: set bit in L2_Position then chip2.writePort(0,L2_Position);
    */

   if (state == Extend) {                    // Defined LOW = Extend = 0
      bitClear(L2_Position,_boxNum);         // Set bit to 0
      TStamp tStamp = {_boxNum, '~', millis() - _startTime, 1, 1};
      printQueue.push(&tStamp);    
   }
   else {                                    // Defined HIGH = Retract = 1
      bitSet(L2_Position,_boxNum);           // Set bit to 1
      TStamp tStamp = {_boxNum, ',', millis() - _startTime, 0, 1};
      printQueue.push(&tStamp);
   }
   chip2.writePort(0,L2_Position);
}

void Box::switchStim1(boolean state) {
   /*   chip0, pins 8..15 map to boxNum 0..7 Lever 1 LED
    *   L1 LED CheckBox is index 3
    *   Defined On = true; Off = false
    */

    if (state == On) {                        // Defined On = true = 1
          bitClear(L1_LED_State,_boxNum);     // LED is on when bit = 0 
          TStamp tStamp = {_boxNum, 'S', millis() - _startTime, 1, 3};
          printQueue.push(&tStamp);
    }
    else {                                    // Defined Off = false = 0
          bitSet(L1_LED_State,_boxNum);       // LED os off when bit is 1
          TStamp tStamp = {_boxNum, 's', millis() - _startTime, 0, 3};
          printQueue.push(&tStamp);
    }
    chip0.writePort(1,L1_LED_State); 
}

void Box::switchStim2(boolean state) {
   /*   chip2, pins 8..15 map to boxNum 0..7 Lever 2 LED
    *   L2 LED CheckBox is index 4
    *   Defined On = true; Off = false
    *   
    *   The LED on lever two (HD) is entirely controlled in checkLeverTwoBits(). 
    *   For all intents and purposes, L2_LED_State mirrors portTwoValue.
    *   
    *   The only exception is when checking or unchecking the checkboxes on the 
    *   Diagnostics page in SA300.py whereupon the following 
    *   code is called.
    *   
    */

   if (state == On) {                        // Defined On = true = 1
      bitClear(L2_LED_State,_boxNum);        // LED is on when bit = 0
      TStamp tStamp = {_boxNum, 'C', millis() - _startTime, 1, 4};
      printQueue.push(&tStamp);
   }
   else {                                   // Defined Off = false = 0
      bitSet(L2_LED_State,_boxNum);       // LED os off when bit is 1
      TStamp tStamp = {_boxNum, 'c', millis() - _startTime, 0, 4};
      printQueue.push(&tStamp);
   }
   chip2.writePort(1,L2_LED_State);
}

void Box::startSession() {
  // Python protocol list:  
  // ['0: Do not run', '1: FR', '2: FR x 20', '3: FR x 40', 
  // '4: PR', '5: TH', '6: IntA: 5-25', '7: Debug', '8: L2 HD']
 
      if (_protocolNum == 1) {           // FR(N)
        _startOnLeverOne = true;
        _blockDuration = _blockDurationInit;
        _maxBlockNumber = 1;
        _IBIDuration = 0;
        _schedPR = false;
        _schedTH = false;
        _maxTrialNumber = 999;           
        _responseCriterion = _paramNum;
        _PRstepNum = 1;                 // irrelevant
        _timeOutDuration = _pumpDuration;
      }
      else if (_protocolNum == 2) {      // FR1 x 20
        _startOnLeverOne = true;
        _blockDuration = _blockDurationInit;
        _maxBlockNumber = 1;
        _IBIDuration = 0;
        _schedPR = false;
        _schedTH = false;
        _maxTrialNumber = 20;
        _responseCriterion = 1;
        _PRstepNum = 1;                 // irrelevant
        _timeOutDuration = 2000;         // 10 mSec x 2000 = 20 sec
      }
      else if (_protocolNum == 3) {      // FR x N
        _startOnLeverOne = true;
        _blockDuration = _blockDurationInit;
        _maxBlockNumber = 1;
        _IBIDuration = 0;
        _schedPR = false;
        _schedTH = false;
        _maxTrialNumber = _paramNum;
        _responseCriterion = 1;
        _PRstepNum = 1;                 // irrelevant
        _timeOutDuration = _pumpDuration;
      }
      else if (_protocolNum == 4) {      // PR(N)
        _startOnLeverOne = true;
        _blockDuration = _blockDurationInit;
        _maxBlockNumber = 1;
        _IBIDuration = 0;
        _schedPR = true;
        _schedTH = false;
        _maxTrialNumber = 999;
        // _responseCriterion  set in startTrial()
        _PRstepNum  = _paramNum;
        _timeOutDuration = _pumpDuration;
      }   
      else if (_protocolNum == 5) {      // TH
        _startOnLeverOne = true;
        // _blockDuration = 21600;       Set in startBlock()
        // _maxTrialNumber = 4;          Set in startBlock()
        _maxBlockNumber = 13;            // 13 blocks
        _IBIDuration = _IBIDurationInit;                                         
        _schedPR = false;
        _schedTH = true;
        _maxTrialNumber = 4;             // Max injections in trial one
        _responseCriterion = 1;
        _PRstepNum = 1;                  // irrelevant
        _timeOutDuration = _pumpDuration;     
      }
      else if (_protocolNum == 6) {      // IntA 5-25 6h
        _startOnLeverOne = true;
        _blockDuration = 300;            // 60 seconds * 5 min
        _maxBlockNumber = 12;            // 12 blocks        
        _IBIDuration = 1500;             // 25 * 60 sec = 1500 seconds
        _schedPR = false;
        _schedTH = false;
        _maxTrialNumber = 999;
        _responseCriterion = 1;
        _PRstepNum = 1;                 // irrelevant
        _timeOutDuration = _pumpDuration; 
      }
      else if (_protocolNum == 7) {     // Flush
        _startOnLeverOne = true;        // irrelevant
        _blockDuration = _blockDurationInit;
        _maxBlockNumber = _paramNum;    // Maybe 24
        _IBIDuration = 0;
        _schedPR = false;                 
        _schedTH = false;
        _maxTrialNumber = 999;
        _responseCriterion = 999;       
        _PRstepNum = 1;                 // irrelevant
        _timeOutDuration = _pumpDuration; 
      }
      else if (_protocolNum == 8) {          // L2 HD  - one HD session
        _startOnLeverOne = false;            // Start on HD Lever
        _HD_Duration = 0;                    // _HD_Duration only used in 2L-PR-HD
        _blockDuration = _blockDurationInit; // seconds - set by OMNI INI Tab
        _maxBlockNumber = 1;
        _IBIDuration = 0;                
        _schedPR = false;                 
        _schedTH = false;
        _maxTrialNumber = 999;          // irrelevant
        _responseCriterion = 999;       // irrelevant
        _PRstepNum = 1;                 // irrelevant
        _timeOutDuration = _pumpDuration; // irrelevant
      }
      else if (_protocolNum == 9) {     // IntA-HD 
        _startOnLeverOne = false;       // Start on HD Lever
        _HD_Duration = 0;               // _HD_Duration only used in 2L-PR-HD
        _blockDuration = _blockDurationInit;  // seconds - set by OMNI in INI Tab
        _maxBlockNumber = 12;
        _IBIDuration = _IBIDurationInit;
        // _IBIDuration = 1500;            // 25 min * 60 sec = 1500 seconds
        _schedPR = false;                 
        _schedTH = false;
        _maxTrialNumber = 999;          // irrelevant
        _responseCriterion = 999;       // irrelevant
        _PRstepNum = 1;                 // irrelevant
        _timeOutDuration = _pumpDuration; // irrelevant
      }
      else if (_protocolNum == 10) {    // 2L-PR-HD
        _startOnLeverOne = true;
        _2L_PR = true;
        _unitDose = false;         
        _HD_Duration = _paramNum * 100;             // 20 seconds
        _blockDuration = _blockDurationInit;  // Session length set by OMNI in INI Tab 
        _maxBlockNumber = 1;
        _IBIDuration = 0;
        _schedPR = true;                 
        _schedTH = false;
        _maxTrialNumber = 999;          // irrelevant
        _responseCriterion = 1;         
        _PRstepNum = 1;                 
        _timeOutDuration = _pumpDuration; // irrelevant
      }   
  if (_protocolNum == 0) endSession();
  else {
      _startTime = millis();
      //  TStamp tStamp = {_boxNum, 'B', millis() - _startTime, 0, 9};
      // printQueue.push(&tStamp);
      TStamp tStamp1 = {_boxNum, 'M', millis(), 0, 9};    // Added to datafile
      printQueue.push(&tStamp1);
      TStamp tStamp2 = {10, 'M', millis(), _boxNum, 0};   // Added to sessionLog
      printQueue.push(&tStamp2);
      _blockNumber = 0;  
      _pumpTime = 0; 
      _timeOutTime = 0;   
      TStamp tStamp3 = {_boxNum, 'G', millis() - _startTime, 0, 9};  // Added to datafile
      printQueue.push(&tStamp3);
      startBlock(); 
  }
  bitSet(boxesRunning,_boxNum);
}

void Box::endSession() { 
    // endTrial(); the only thing this did was retract the lever, but see next line. 
    moveLeverOne(Retract);         
    switchStim1(Off);
    switchTimedPump(Off);
    _pumpTime = 0;
    _timeOutTime = 0;
    _boxState = FINISHED;    
    TStamp tStamp1 = {_boxNum, 'E', millis() - _startTime, 0, 9};
    printQueue.push(&tStamp1);
    TStamp tStamp2 = {10, 'm', millis(), _boxNum, 0};   // Added to sessionLog
    printQueue.push(&tStamp2);
    moveLeverTwo(Retract);
    bitClear(boxesRunning,_boxNum);
}

void Box::tick() {                        // do stuff every 10 mSec 
    if (_timedPumpOn) {
       _pumpTime++;
       if (_pumpTime >= _pumpDuration) switchTimedPump(Off);
    }  
    if (_boxState == L1_TIMEOUT) {
       _timeOutTime++;
       if (_timeOutTime == _timeOutDuration) endTimeOut();     
    }
    _tickCounts++; 
    if (_tickCounts == 100)    {         // do this every second
       _tickCounts = 0;             
       if (_boxState == L1_ACTIVE || _boxState == L1_TIMEOUT || _boxState == L2_HD) {
            _blockTime++;         
            TStamp tStamp = {_boxNum, '*', _blockTime, 0, 9};
            printQueue.push(&tStamp);
            if (_blockTime == _blockDuration) {
              if (_boxState == L2_HD) endHDTrial();
              else endBlock();
            }
       }
       else if (_boxState == IBI) {
            _IBITime++;
            TStamp tStamp = {_boxNum, '*', _IBITime, 0, 9};
            printQueue.push(&tStamp);
            if (_IBITime >= _IBIDuration) endIBI(); 
            }      
       } 
       if (_boxState == L2_HD && _2L_PR) {
          _HDTime++;
          if (_HDTime >= _HD_Duration) endHDTrial();
       } 
}

void Box::handle_L1_Response() { 
   if (_boxState == L1_ACTIVE) { 
         TStamp tStamp = {_boxNum, 'L', millis() - _startTime, 0, 9};
         printQueue.push(&tStamp);
         _trialResponses++;
         if (_trialResponses >= _responseCriterion) {
                 endTrial();
                 reinforce();
         }
    }  
}

void Box::handle_L2_Response(byte state) {
   /* Handles timestamp only, 
      HD control of pump and LED handled in checkLeverOneBits()
    */

   if (state == 1) {
      TStamp tStamp1 = {_boxNum, 'h', millis() - _startTime, 0, 9};
      printQueue.push(&tStamp1);
   }
   else {
      TStamp tStamp2 = {_boxNum, 'H', millis() - _startTime, 1, 9};
      printQueue.push(&tStamp2);
   } 
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
  _blockDurationInit = blockDuration;
}

void Box::setIBIDuration(int IBIDuration)   {
  _IBIDurationInit = IBIDuration;
}

void Box::reportParameters() {
 /*  _boxNum
 *   _protocolNum
 *   _paramNum
 *   _blockDurationInit
 *   _IBIDurationInit
 *   _pumpDuration
 *   _maxTrialNumber 
 *   
 *   _responseCriterion
 *   _PRstepNum
 *   _timeOutDuration
 *   _maxTrialNum
 */
  Serial.println("9 *****BOX_"+String(_boxNum)+"****");
  Serial.println("9 _paramNum:"+String(_paramNum));
  Serial.println("9 _protocolNum:"+String(_protocolNum));
  Serial.println("9 _blockDuration:"+String(_blockDurationInit)+"sec");
  Serial.println("9 _IBIDurationInit:"+String(_IBIDurationInit)+"sec");
  Serial.println("9 _IBIDuration:"+String(_IBIDuration)+"sec");
  Serial.println("9 _HD_Duration:"+String(_HD_Duration)+"sec");
  Serial.println("9 _pumpDuration:"+String(_pumpDuration)+"0mSec");
  Serial.println("9 _responseCriterion:"+String(_responseCriterion));
  Serial.println("9 _PRstepNum:"+String(_PRstepNum));
  Serial.println("9 _timeOutDuration:"+String(_timeOutDuration)+"0mSec");
  Serial.println("9 _maxTrialNumber:"+String(_maxTrialNumber));
  Serial.println("9 _maxBlockNumber:"+String(_maxBlockNumber));
} 

void Box::getBlockTime() {
  // Serial.println("9 "+String(_boxNum)+" BTime: "+ String(_blockTime));
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
// Note that SysTick is a good alternative and has been tested with the M4
// **************************************************************************
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

void resetChips() {
   chip0.begin();
   chip1.begin();
   chip2.begin();
   chip3.begin();
   for (uint8_t i = 0; i <= 15; i++) {
      chip0.pinMode(i,OUTPUT);             // Set chip0 to OUTPUT - Retract L1 and LED1
      chip2.pinMode(i,OUTPUT);             // Set chip2 to OUTPUT = Retract L2 and LED2
   }
   for (uint8_t i = 0; i <= 7; i++)  {
      chip1.pinMode(i,INPUT_PULLUP);       // inputs on lower register   
      chip3.pinMode(i,INPUT_PULLUP);          
   }
   for (uint8_t i = 8; i <= 15; i++) {
      chip1.pinMode(i, OUTPUT);            // outputs on higher register Pump   
      chip3.pinMode(i, OUTPUT);            // AUX
   }
   chip0.writePort(0,L1_Position);         // To whatever state has previously been assigned
   chip0.writePort(1,L1_LED_State);         
   chip2.writePort(0,L2_Position);
   chip2.writePort(1,L2_LED_State);
   checkLever1 = true;
   checkLever2 = true; 
   TStamp tStamp = {10, '&', millis(), 0, 0};        // Added to sessionLog
   printQueue.push(&tStamp);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect.
  }
  pinMode(LED_BUILTIN, OUTPUT);   // GPIO 13
  L1_Position = 0xFF;             // Retract L1
  L1_LED_State = 0xFF;            // L1 LED Off
  L2_Position = 0xFF;             // Retract L2
  L2_LED_State = 0xFF;            // L2 LED Off
  pumpState = 0x00;               // Pumps Off
  pumpStateL1 = 0x00;
  pumpStateL2 = 0x00;
  resetChips();
  delay(500); 
  init_10_mSec_Timer(); 
  Serial.println("9 Ver=301.01");
}

void enterSafeMode() {
  if (Verbose) Serial.println("Disabling_Outputs");
  TStamp tStamp = {10, '(', millis(), 0, 0};        // Added to sessionLog
  printQueue.push(&tStamp); 
  // ***** Switch all output ports OFF *****
  L1_Position = 0xFF;              // Retract L1
  chip0.writePort(0,L1_Position);  
  L1_LED_State = 0xFF;             // L1 LED Off
  chip0.writePort(1,L1_LED_State);
  L2_Position = 0xFF;              // Retract L2
  chip2.writePort(0,L2_Position);
  L2_LED_State = 0xFF;             // L2 LED Off
  chip2.writePort(1,L2_LED_State);
  pumpState = 0x00; 
  pumpStateL1 = 0x00;
  pumpStateL2 = 0x00;
  chip1.writePort(1,pumpState);      // Pumps Off
  chip3.writePort(1,0xFF);           // Aux Off
  // ***** Configure all output ports to inputs *****  
  for (uint8_t i = 0; i <= 15; i++) {
      chip0.pinMode(i,INPUT);             // Set chip0 to OUTPUT
      chip2.pinMode(i,INPUT);             // Set chip2 to OUTPUT
  }
  for (uint8_t i = 8; i <= 15; i++) {
     chip1.pinMode(i, INPUT);               
     chip3.pinMode(i, INPUT);               
  }
  checkLever1 = false;
  checkLever2 = false; 
}

void handleOutputError(){
   /* Report output errors; then check ten times to see if it can recover.  
      If it can't recover then endSession. Otherwise carry on...
   */
   boolean errorFound = false;
   boolean recoveredFromError = false;
  
   if (Verbose) Serial.println("9 "+String(millis())+" Output Error");
      
   for (int x = 0; x < 10; x++) {
      boolean errorFound = false;
      if (L1_Position  != chip0.readPort(0)) {
         Serial.println("10 $ "+String(millis())+" 1 0");
         errorFound = true;
         chip0.writePort(0,L1_Position);   
      }
      if (L1_LED_State != chip0.readPort(1)) {
         Serial.println("10 $ "+String(millis())+" 2 0");
         errorFound = true;
         chip0.writePort(1,L1_LED_State);
      }
      if (pumpState    != chip1.readPort(1)) {
         Serial.println("10 $ "+String(millis())+" 3 0");
         errorFound = true;
         chip1.writePort(1,pumpState);
      }
      if (L2_Position  != chip2.readPort(0)) {
         Serial.println("10 $ "+String(millis())+" 4 0");
         errorFound = true;
         chip2.writePort(0,L2_Position);
      }
      if (L2_LED_State != chip2.readPort(1)) {
         Serial.println("10 $ "+String(millis())+" 5 0");
         errorFound = true;
         chip2.writePort(1,L2_LED_State);
      }
      if (!errorFound) {
         Serial.println("10 % "+String(millis())+" 5 0");  
         recoveredFromError = true;
         break;
      }
  }
  if (!recoveredFromError) {      
      resetChips();                                  // try one last time after reset
      if (checkOutputPorts()) {
         abortSession();        
      }
      else {
         Serial.println("10 r "+String(millis())+" 0 0");   // increment self.outputRecoveries 
      } 
   }
}

boolean checkOutputPorts() {
    boolean errorFound = false;
    if (L1_Position  != chip0.readPort(0)) errorFound = true;
    if (L1_LED_State != chip0.readPort(1)) errorFound = true;
    if (pumpState    != chip1.readPort(1)) errorFound = true;
    if (L2_Position  != chip2.readPort(0)) errorFound = true;
    if (L2_LED_State != chip2.readPort(1)) errorFound = true;
    return errorFound;
}

void startSession(int boxNum) {
   boxArray[boxNum].startSession(); 
   // sessionLog will show start time for each box.  
}

void endSession(int boxNum) {
   boxArray[boxNum].endSession();
   // sessionLog will show start time for each box.
}

void abortSession() {  
   for (byte bits = 0; bits < 8; bits++) {
      if ((boxesRunning & (1 << bits)) > 0) {      // check which boxes running
         boxArray[bits].endSession();         
      }
   }
   enterSafeMode();
   sessionAborted = true; 
   boxesRunning = 0;
   TStamp tStamp = {10, '!', millis(), 0, 0};
   printQueue.push(&tStamp);
}



/*    ********************** Checking Inputs *********************************
checkLeverOneBits() and checkLeverOneBits() were modified to filter out noise. 
I appears that on occassion, a ground spike would flip all bits and the program 
would interpret it as simultaneous responses on all boxes. The sketch now 
scans the input port to see if the register has gone to 0x00. If so, it calls 
handleInputError(byte leverNum, byte portValue)
*/

void handleInputError(byte leverNum, byte portValue) {
   boolean recoveredFromError = false;   
   byte _portValue;
   
   Serial.println("10 @ "+String(millis())+' '+String(leverNum)+' '+String(portValue));

   for (int x = 0; x < 10; x++) {                         // Try ten times
      if (leverNum == 1) _portValue = chip1.readPort(0);
      else _portValue = chip3.readPort(0);
      if (_portValue != 0) {
         recoveredFromError = true;
         break;
      }
   }                                                  // If error after 10 tries 
   if (recoveredFromError) {
       Serial.println("10 # "+String(millis())+' '+String(leverNum)+' '+String(_portValue));
   }
   else {
      Serial.println();
      resetChips(); 
      if (chip1.readPort(0) != 0 && chip3.readPort(0) != 0) {
         Serial.println("10 # "+String(millis())+' '+String(leverNum)+' '+String(_portValue));
      }
      else {
         abortSession();
      }
   }   
}


void checkLeverOneBits() {
/*
 * boxArray[i].handle_L1_Response() is called when the lever goes to ground.
 */
  
   static byte oldPortOneValue = 255;       
   portOneValue = chip1.readPort(0);
   if (portOneValue == 0) handleInputError(1,portOneValue);   // Check for Error 
   else if(portOneValue != oldPortOneValue) {                 // something new
      oldPortOneValue = portOneValue;
      // Serial.println (portOneValue,BIN);
      for (byte i = 0; i < 8; i++) {
         newLeverOneState[i] = bitRead(portOneValue,i);
            if (newLeverOneState[i] != lastLeverOneState[i]) {          
                lastLeverOneState[i] = newLeverOneState[i]; 
                if (newLeverOneState[i] == 0) {
                   boxArray[i].handle_L1_Response();
                }
            }
        }    
    }           
}

void checkLeverTwoBits() {
  /*  (1 << 7) - This shifts 1 to the left seven bits creating 
   *  a mask = 10000000. Together with the bitwise and (&) it evaluates each 
   *  bit in the byte.
   */
   static byte oldPortTwoValue = 255;
   portTwoValue = chip3.readPort(0);                             // Check for Error
   if (portTwoValue == 0) handleInputError(2,portTwoValue);      // Input OR output Error
   else {
      if (oldPortTwoValue != portTwoValue) {                     // something changed
         for (int bits = 7; bits > -1; bits--) {
            if ((portTwoValue & (1 << bits)) != (oldPortTwoValue & (1 << bits))) {
               if (bitRead(portTwoValue,bits) == 1) {
                  boxArray[bits].handle_L2_Response(1);
               }
               else {
                  boxArray[bits].handle_L2_Response(0);
               }                                  
            }        
         }
      L2_LED_State = portTwoValue;              // mirror pump state
      chip2.writePort(1,L2_LED_State); 
      }
      // Configure pumpState 
      oldPortTwoValue = portTwoValue;
      pumpStateL2 = (255-portTwoValue);
      pumpState = (pumpStateL1 | pumpStateL2);  // bitwise OR
      // chip1.writePort(1,pumpState);          // Called as next instruction in tick()
   }
}   
 
void decodeSysVars(byte varCode) {
     /* Test of decoding
    * Uses "left shift" of 1 to generate a mask
    * Could have used 1,2,4,8 etc
    */
   if ((varCode & (1 << 0)) > 0) {checkLever1 = true;
      if (showDebugOutput) Serial.println("9 checkLever1=true");
   }
   else {checkLever1 = false;  
      if (showDebugOutput) Serial.println("9 checkLever1=false");
   }
   
   if ((varCode & (1 << 1)) > 0) {checkLever2 = true;
      if (showDebugOutput) Serial.println("9 checkLever2=true");
   }
   else {checkLever2 = false;  
      if (showDebugOutput) Serial.println("9 checkLever2=false");
   }
   if ((varCode & (1 << 2)) > 0) {checkOutputs = true;
      if (showDebugOutput) Serial.println("9 checkOutputs=true");
   }
   else {checkOutputs = false;  
      if (showDebugOutput) Serial.println("9 checkOutputs=false");
   }   
   if ((varCode & (1 << 3)) > 0) {abortEnabled = true;
      if (showDebugOutput) Serial.println("9 abortEnabled=true");
   }
   else {showMaxDelta = false;  
      if (showDebugOutput) Serial.println("9 abortEnabled=false");
   }
   if ((varCode & (1 << 4)) > 0) {showMaxDelta = true;
      if (showDebugOutput) Serial.println("9 showMaxDelta=true");
   }
   else {showMaxDelta = false;  
      if (showDebugOutput) Serial.println("9 showMaxDelta=false");
   }
   if ((varCode & (1 << 5)) > 0) {Verbose = true;
      if (showDebugOutput) Serial.println("9 Verbose=true");
   }
   else {Verbose = false;  
      if (showDebugOutput) Serial.println("9 Verbose=false");
   }
   if ((varCode & (1 << 6)) > 0) {showDebugOutput = true;
      if (showDebugOutput) Serial.println("9 showDebugOutput=true");
   }
   else {showDebugOutput = false;  
      if (showDebugOutput) Serial.println("9 showDebugOutput=false");
   }
}

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
     if (stringCode == "chip0") chip0.digitalWrite(num1,num2); 
     else if (stringCode == "G")     startSession(num1);
     else if (stringCode == "Q")     endSession(num1);
     else if (stringCode == "L1")    boxArray[num1].handle_L1_Response(); 
     else if (stringCode == "P")     boxArray[num1].switchTimedPump(On);
     else if (stringCode == "PROTOCOL") boxArray[num1].setProtocolNum(num2);
     else if (stringCode == "PARAM") boxArray[num1].setParamNum(num2);
     else if (stringCode == "TIME")  boxArray[num1].setBlockDuration(num2);  
     else if (stringCode == "IBI")   boxArray[num1].setIBIDuration(num2);         
     else if (stringCode == "PUMP")  boxArray[num1].setPumpDuration(num2); 
     else if (stringCode == "R")     boxArray[num1].reportParameters();
     else if (stringCode == "=")     boxArray[num1].moveLeverOne(Extend);     // extend lever1
     else if (stringCode == ".")     boxArray[num1].moveLeverOne(Retract);    // retract lever1
     else if (stringCode == "~")     boxArray[num1].moveLeverTwo(Extend);  
     else if (stringCode == ",")     boxArray[num1].moveLeverTwo(Retract);
     else if (stringCode == "s")     boxArray[num1].switchStim1(Off);
     else if (stringCode == "S")     boxArray[num1].switchStim1(On);
     else if (stringCode == "c")     boxArray[num1].switchStim2(Off);
     else if (stringCode == "C")     boxArray[num1].switchStim2(On);     
     else if (stringCode == "D")     reportDiagnostics(); 
     else if (stringCode == "Abort") abortSession();
     else if (stringCode == "r")     resetChips();
     // else if (stringCode == "A")     Serial.println("10 A "+String(millis())+" 0 0");  // On connect
     else if (stringCode == "SYSVARS")  decodeSysVars(num1);
     else if (stringCode == "O"){
          if (checkOutputPorts()) handleOutputError();
          }
     // debug stuff
     else if (stringCode == "i")     timeUSB();
     else if (stringCode == "E")     echoInput = !echoInput;   
     inputString = "";
   }
}

int freeRam () {
  char stack_dummy = 0;
  return &stack_dummy - sbrk(0);
}

void reportDiagnostics() {
   Serial.println("9 maxDelta="+String(maxDelta));
   Serial.println("9 maxQueueRecs="+String(maxQueueRecs));
   Serial.println("9 freeRam="+String(freeRam()));
   Serial.println("9 portOneValue="+String(portOneValue));
   Serial.println("9 portTwoValue="+String(portTwoValue));
   Serial.println("9 boxesRunning="+String(boxesRunning,BIN));
   Serial.println("9 showDebugOutput="+String(showDebugOutput));
   Serial.println("9 Verbose="+String(Verbose));
   Serial.println("9 showMaxDelta="+String(showMaxDelta));
   Serial.println("9 abortEnabled="+String(abortEnabled));
   Serial.println("9 checkOutputs="+String(checkOutputs));
   Serial.println("9 checkLever2="+String(checkLever2));
   Serial.println("9 checkLever1="+String(checkLever1));   
   Serial.println("9 "+verStr);
   maxDelta = 0;  
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
   static byte sec = 0;
   int boxIndex;
   unsigned long delta, micro1;
   micro1 = micros();
   // micro1 = millis();
   tickCounts++;
   if (tickCounts >= 100) {                           // every second
       digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
       if (checkOutputs){
          if (checkOutputPorts()) handleOutputError();        
       }
       sec++;
       tickCounts = 0;
   }
   if (!sessionAborted) {
     for (uint8_t i = 0; i < 8; i++) boxArray[i].tick();
     if (checkLever1) checkLeverOneBits();
     if (checkLever2) checkLeverTwoBits(); 
     
     pumpState = (pumpStateL1 | pumpStateL2);  // bitwise OR
     chip1.writePort(1,pumpState);
  
     L2_LED_State = portTwoValue;
     chip2.writePort(1,L2_LED_State);
   }
   
   getInputString();
   
   sendOneTimeStamp();
   delta = micros() - micro1;
   // delta = millis() - micro1;
   if (delta > maxDelta) maxDelta = delta;
   if (tickCounts == 99){
      if (showMaxDelta) {
         Serial.println("9 maxDelta(uSec)="+String(maxDelta));
         maxDelta = 0; 
      }
   }   
}

void loop() {
   if (tickFlag) {
      tickFlag = false;
      tick();
   }
}
