/*  
 *   
 *   Nov 23, 2020 
 *   
 *   Changes to this version (after testing it will be version 301.00)
 *   
 *   Changes to the way outputs are controlled: 
 *   -  Old way: bits were flipped using chip0.digitalWrite(bit,state) 
 *   -  New way: flip bit in variable (eg. L1_Position) then chip0.writePort(0, L1_Position)
 *   
 *   Procedures rewritten:
 *   Box::moveLeverOne(int state) 
 *   Box::moveLeverTwo(int state)
 *   Box::switchStim1(boolean state)
 *   Box::switchStim2(boolean state)
 *   Box::switchTimedPump(On / Off)
 *   
 *   handleInputError(byte leverNum, byte portValue)
 *   checkLeverOneBits() updated and now calls handleInputError()
 *   
 *   Box::switchRewardPortOn(boolean timed) RENAMED to switchTimedPump(On)
 *        _timedRewardOn RENAMED _timedPumpOn
 *        _rewardTime RENAMED to _pumpTime
 *        _rewardDuration RENAMED to _pumpDuration
 *        rewardDuration RENAMED to pumpDuration
 *   
 *   sysVar - not need for at the moment; DecodeSySVars(num) commented out
 *       <SYSVARS num> and DecodeSySVars(num) sets eight sysVars
 *       Change labels for sysVars0-2 in Python
 *       0 Reward Port - Pumps (0) AUX (1)
 *       1 Reward Logic - 5VDC on (0) GND On (1)
 *       2 Check Lever 2 - False (0) True (1)
 *       3 - 7 unused
 *       
 *   SA200.py - sysVar labels removed
 *   
 *   Box::deliverFoodPellet() stub added 
 *      see notes for future To Do
 *      
 *   "M" timestamp is the millis() at session _startTime
 *   
 *   handleInputError() sends "!" with millis() timestamp 
 *   Presumably, Analysis can use "M" and "!" to display input errors. 
 *   "!", "^"     < timestamps for input and output errors
 *   
 *   handleOutputError() will send "^"
 *   
 *   
 *        
 *   ISSUES and Priorities
 *   
 *   Check "M" and "G" timestamps. 
 *  
 *   Check Documentation, SA200.py Programmers Guide, API, codes 
 *   
 *   
 *   
 *   Rename SA200.py to SA300.py
 *   Clean up this mess. If necessary, move stuff to Documentation
 *   
 *   add chip1.writePort(1,pumpState) to tick()
 *   
 *   
 *   1. Re-assess 
 *      Old version: 
 *      checkLeverOneBits() calls Box::handle_L1_Response();
 *      checkLeverTwoBits() calls Box::handle_L2_Response(byte state);
 *      
 *      For better or worse, checkLeverTwoBits() handles pump and timestamps for
 *        the HD lever.
 *        
 *      Think this through and document it in Powerpoint flow chart
 *   
 *   2. Configure pumpState, pumpStateL1 and pumpStatel2  
 *   
 *      Get a handle on pumpstate etc.
 *   
 *    pumpStateL1                             <- determined in Box::switchRewardPortON / Off()
 *    pumpStateL2 = (255-portTwoValue);       <- deteremined in Box::checkLeverTwoBits()
 *   
 *    pumpState = (pumpStateL1 | pumpStateL2);  // bitwise OR
 *    chip1.writePort(1,pumpState);           <- could happen at end of every tick.
 *   
 *   
 *   3. rewrite checkLeverOneBits() checkLeverTwoBits()
 *   
 *   4. Need leverTwoExists??
 *      The issue is when and whether to call checkLeverTwoBits() 
 *      Define global variable that is set by any box running HD
 *   
 *   replace turnStuffOff() with disableOutputs()  ????
 *   
 *   HD_DEMO.ino uses startSesssion() and endSession()
 *   Here, _startSession is local to Box
 *   -  Move stuff to startup() ?
 *       
 *   
 *   
 *   If not running HD and therefore not checking InputPort2) there is little 
 *   overhead to worry about. 
 *   
 *   This is done every 10 mSec in checkInputPort2()
 *   - Issue - will this be called regardless of schedule?
 *      Or only when L2 is extended? 
 *    
 *   
 *   
 *   Send System timestamps - eg. "!" input or output error
 *   - Python should add it to every datafile.
 *   - Raises issue of system time vs box time
 *   
 *   Println statements (if Verbose) need to be changed to some sort
 *   of timestamp
 *   
 *   lastLeverTwoState and newLeverTwoState still needed? 
 *   
 *   rename "Verbose" - "verbose" is a reserved word
 *   
 *   
 *   Added
 *   states { PRESTART, L1_ACTIVE, L1_TIMEOUT, IBI, L2_HD, FINISHED };
 *   
 *   self.sched = 
 *   ['0: Do not run', '1: FR(N)', '2: FR1 x 20 trials', '3: FR1 x N trials', '4: PR(step N)', 
 *   '5: TH', '6: IntA: 5-25', '7: Flush', '8: L2-HD', '9: IntA-HD', '10: 2L-PR-HD']
 *   
 *   _timeOut deleted
 *   inactiveLeverExists renamed to leverTwoExists 
 *   
 *   
 *   moveLeverTwo(Extend or Retract) as appropriate for protocol 
 *   
 *   
 *   July 14, 2020
 *   
 *   First iteration of Ver 300 - rethink of the Box Class and 
 *   downsizing of Lever Class
 *   
 *   checkLeverTwoBits() should pass the change in state and print it to the Serial Port.
 *   
 *   To Do:
 *   
 *   Rename and differentiate procedures for L1 and L2 (moveLever1 etc)
 *   checkLeverTwoBits(): Implement the same check for phantom responses as checkLeverOneBits()
 *   
 *   See Documents/SelfAdmin/Sketch FlowChart.ppt for program flow
 *   
 *   
 *   May 9th.
 *   
 *   TH - Block one ends after 4 injections
 *   "Flush" - N injections separated by Block Time
 *   _cycle stuff deleted
 *   sessionRunning deleted
 *   Fixed lever retract timestamp
 *   boolean _pumpOn = added to functionally separate pump and _timeOut 
 *   Suppress initialize report
 *   Only responds to lever response during BLOCK
 *   Conflict of commands and timestamps for Lever resolved (=,.) 
 *   reportParameters() on Feather called by reportParameters() in Python
 *   
 *   Update Adafruit SAMD Boards to Ver 1.2.7
 *  
 *   To Do:
 *   
 *   Document SysVar stuff
 *   
 *   Document the fact that the lever timestamp is/was inconsistent 
 *   (= and -) vs (= and.) and may require a work around in Analysis
 *   
 *   Block ending retracts lever etc. but no Trial end timestamp. 
 *   endBlock() should endTrial() 
 *   
  
 *   
 *   Check 5-25 - should it override all parameters?
 *   
 *   What should be handled by a sysVar?
 *   -  CheckLeverOneBits?
 *   -  
 *   
 *   _blockDuration and _IBIDuration are a bit complicated.
 *   Some protocols have them set by default (eg.IntA 5-25)
 *   But they can be set in TH.
 *   - the complication is that the first block duration is 6h and changes 
 *   in the second block.
 *   
 *   So we need _blockDurationInit and _IBIDurationInit which can be 
 *   used or not in specific protocols. 
 *   
 * 
 *    
 *    
 *   To Do:
 *   SwitchRewardDevice(On) etc. 
 *    Time stamp could reflect Pump (P,p) or Hopper (H,h)
 *    But that would require updates to graphs
 *   
 *  
 *   
 *   Done on April 7th
 *   Defined an array of eight boolena variables.
 *   twoLever removed
 *   debugBoolVarList removed
 *   sysVarArray[1] substituted for pumpOnHigh
 *   
 *   Document how the timing is done and relate it to the PowerPoint slide.
 *    Reinforce() changes _timeOut = true which is checked in tick() each 10 mSec.
 *   - use only one _rewardTime
 *   - _timeOut can be different and longer than pumpDuration
 *   
 *   Test:
 *   - The Python time should reflect _blockTime or _IBITime
 *   - How should IBI be indicated?
 *   
 *   ************************************************************
 *   Four chips are initiaized by the program
 *   
 *   Each bank of levers is associated with 2 (16 bit) ports. 
 *   And, for example, port1.read() and port0.write are using 16 bit addresses. The
 *   MCP23S17 class allows addressing using 8 bit addresses (so don't have to add 8).
 *   It would be more transparent if individual ports were assigned to the lever.  
 *   Also, right now the input and reinforcer output is on the same chip. Need to be  
 *   able to choose chip and port _inputAddr(0,0)
 *   
 *   void Lever::switchRewardPort(boolean state) {
        chip1.digitalWrite(_boxNum+8,!state);
 *   
 *   Default:  
 *    _lowChip = chip0;
 *    _highChip = chip1; 
 *   
 *   lever.setPorts(inputPort, outputPort).
 *      _inputPort = inputPort;
 *   *************************************************************
 *   
 *   
 *   1. The lever shouldn't care whether it is a pump 
 *   or hopper is switched. It could be coupled to only one device and the way the
 *   port is selected would determine what is switched.
 *   
 *   2. The lever has two options and which reward is used is software selectable. 
 *   
 *   Option two is quicker. 
 *     
 *   Note that all timestamps come from Lever.
 *   The lever could own the Queue and Box could pull from it. 
 *   If Box needs to send a message, just use Serial.println()
 *   
 *   Implications:
 *   Timestamp codes will have to be instantiated differently for lever1 and lever2 
 *   
 *   TIMEOUT and LED = timeOutDuration
 *   pumpDuration may be shorter
 *   
 *   Changes: 
 *   _pumpTime++ until equals pumpDuration
 *   _blockTime++ until equals _blockDuration
 *   _IBILength changed to _IBIDuration
 *   states _boxState = PRESTART; 
 *   
 *   _blockTime was used for IBI as well - created _IBITime
 *     
 * This should handle eight boxes with or without an inactive lever.
 * 
 * Two MCP23S17 port expanders are controled by the MCP23S17 library 
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

const uint8_t chipSelect = 10;  // All four chips use the same SPI chipSelect
MCP23S17 chip0(chipSelect, 0);  // Instantiate 16 pin Port Expander chip at address 0
MCP23S17 chip1(chipSelect, 1);  
MCP23S17 chip2(chipSelect, 2);    
MCP23S17 chip3(chipSelect, 3); 

byte portOneValue = 255, portTwoValue = 255;

// Variables needed for error detection - from HD_DEMO.ino
// The following variables are used to track the state of each output port

boolean Verbose = true;   // note that "verbose" (small v) is a reserved word

// Output Ports Values
byte L1_Position = 0xFF;
byte L1_LED_State = 0xFF;      
byte pumpState = 0x00; 
byte pumpStateL1 = 0x00;        // Determined by lever 1
byte pumpStateL2 = 0x00;        // Determined by lever 2 (HD)
byte L2_Position = 0xFF;        // Retracted
byte L2_LED_State = 0xFF;       // Off
int inputErrors = 0;
int inputRecoveries = 0;
int outputErrors = 0;
int outputRecoveries = 0;

boolean sysVarArray[8] = {false,false,false,false,false,false,false,false};
// sysVarArray[0] used for reward type; 0 = Drug, 1 = Food
// sysVarArray[1] used for logic type: 0 = 5VDC switches On, 1 = GND switches On
// sysVarArray[2] used for leverTwoExists

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
char pumpCharArray[3] = "pP";  // used for timestamp
char lever2CharArray[3] = "jJ"; 

volatile boolean tickFlag = false;

// program housekeeping varaibles
String inputString;
boolean echoInput = false;
unsigned long maxDelta = 0;
byte maxQueueRecs = 0;
byte diffCriteria = 1;

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
      _pumpTime = 0;
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
    _pumpTime = 0;
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
      Originally sysVarArray[1] was designed to select drug vs food
      Note that drug vs food uses different ports and 5VDC vs GND to switch ON
      Future use will likely depend on a protocol and local variables 
      determining whether to call switchPumpOn() or deliverFoodPellet() 
      timestamp Hopper (H,h)
   */  
}

void Box::switchTimedPump(int state) { 
    // boxNum 0..7 maps to pin 0..7 on chip1, pins 8..15 paps to Pumps
    // Pump CheckBox is index 2
    // pumpState = 0x00    <- OFF
    // Pump off with bitClear()
    // Pump On with bitSet() 
    // Old way: chip1.digitalWrite(_boxNum+8,level);
    
    if (state == On) {
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

/*

void Box::switchRewardPortOff() { 
    // boxNum 0..7 maps to pin 0..7 on chip1 or chip3 
    // Normally: On (true) switches the bit to HIGH
    //       and Off (false) switches the bit to LOW
    // BUT if sysVarArray[1] == true -> then the reverse happens
   
    boolean level;
    // sysVarArray[1] selects of the device goes on with 5VDC or GND
    // sysVarArray[1] 5VDC On = 0; GND On = 1 
    if (sysVarArray[1]) level = HIGH;     // if "GND On" go high to turn off
    else level = LOW;                     // else '5VDC On" go low to turn off

    // sysVarArray[0] selects either Pump Port or AUX Port
    if (sysVarArray[0] == 0) chip1.digitalWrite(_boxNum+8,level);
    else chip3.digitalWrite(_boxNum+8,level); 
                    
    TStamp tStamp = {_boxNum, 'p', millis() - _startTime, 0, 2};
    printQueue.push(&tStamp);

    _timedPumpOn = false;
    
    // The Pump CheckBox is index 2 
}

*/

void Box::moveLeverOne(int state) { 
   /*  chip0, pins 0..7 map to boxNum 0..7 to extend/retract Lever 1
    *  Lever1 (active) CheckBox is index 0
    *  Previous version: chip0.digitalWrite(_boxNum,state);
    *  Revision: set bit in L1_Position then chip0.writePort(0,L1_Position);
    */
   
   if (state == Extend) {                 // Defined LOW = Extend = 0 
      bitClear(L1_Position,_boxNum);      // Set bit to 0
      TStamp tStamp = {_boxNum, '.', millis() - _startTime, 0, 0};
      printQueue.push(&tStamp); 
   }
   else {                                 // Defined LOW = Extend = 0
      bitSet(L1_Position,_boxNum);        // Set bit to 1
      TStamp tStamp = {_boxNum, '=', millis() - _startTime, 1, 0};
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

    // Junk
    // boolean level;
    // level = !state;                        // Defined On = true -> level goes low 
    // chip0.digitalWrite(_boxNum+8,level);   // boxNum 0..7 maps to pin 8..15 on chip0

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
    */
    
    // Junk
    // boolean level;
    // level = !state;                        // On = true -> level goes low    
    // chip2.digitalWrite(_boxNum+8,level);   // boxNum 0..7  maps to pin 8..15 on chip2

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
      TStamp tStamp = {_boxNum, 'M', millis(), 0, 9}; 
      printQueue.push(&tStamp);
      _blockNumber = 0;  
      _pumpTime = 0; 
      _timeOutTime = 0;   
      tStamp = {_boxNum, 'G', millis() - _startTime, 0, 9}; 
      printQueue.push(&tStamp);
      startBlock(); 
  }
}

void Box::endSession() { 
    // endTrial(); the only thing this did was retract the lever, but see next line. 
    moveLeverOne(Retract);         
    switchStim1(Off);
    switchTimedPump(Off);
    _pumpTime = 0;
    _timeOutTime = 0;
    _boxState = FINISHED;    
    TStamp tStamp = {_boxNum, 'E', millis() - _startTime, 0, 9};
    printQueue.push(&tStamp);
    moveLeverTwo(Retract);
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

void Box::handle_L2_Response(byte state) {   // HD lever change
  // checkLeverTwoBits passes the state of each bit in the register
  //
  // Serial.println("9 pumpOff="+String(testArray[0]));
  // Serial.println("9 pumpOn="+String(testArray[1]));
  /*if (_boxState == L2_HD) {
      if (state) {
        chip1.digitalWrite(_boxNum+8,0);
        TStamp tStamp1 = {_boxNum, 'J', millis() - _startTime, 0, 9};
        printQueue.push(&tStamp1);
        TStamp tStamp2 = {_boxNum, 'P', millis() - _startTime, 1, 2};
        printQueue.push(&tStamp2);
      }
      else {
        chip1.digitalWrite(_boxNum+8,1);
        TStamp tStamp1 = {_boxNum, 'j', millis() - _startTime, 0, 9};
        printQueue.push(&tStamp1);
        TStamp tStamp2 = {_boxNum, 'p', millis() - _startTime, 1, 2};
        printQueue.push(&tStamp2);
      }
  }*/
  if (_boxState == L2_HD) {
      TStamp tStamp1 = {_boxNum, lever2CharArray[state], millis() - _startTime, 0, 9};
      printQueue.push(&tStamp1);
      TStamp tStamp2 = {_boxNum, pumpCharArray[state], millis() - _startTime, 1, 2};
      printQueue.push(&tStamp2);    
      if (state) {                      // Lever pressed: HIGH (1)
        chip1.digitalWrite(_boxNum+8,0);
        pumpOn = true;
      }
      else {                            // Lever up
        chip1.digitalWrite(_boxNum+8,1);
        pumpOn = false;
      }
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
   if (Verbose) Serial.println("Resetting Port Expander Chips");
   for (uint8_t i = 0; i <= 15; i++) {
      chip0.pinMode(i,OUTPUT);             // Set chip0 to OUTPUT
      chip2.pinMode(i,OUTPUT);             // Set chip2 to OUTPUT
   }
   for (uint8_t i = 0; i <= 7; i++)  {
      chip1.pinMode(i,INPUT_PULLUP);          
      chip3.pinMode(i,INPUT_PULLUP);          
   }
   for (uint8_t i = 8; i <= 15; i++) {
      chip1.pinMode(i, OUTPUT);               
      chip3.pinMode(i, OUTPUT);               
   }
   chip0.writePort(0,L1_Position);         // To whatever state has previously been assigned
   chip0.writePort(1,L1_LED_State);         
   chip2.writePort(0,L2_Position);
   chip2.writePort(1,L2_LED_State); 
}

void disableOutputs() {
  if (Verbose) Serial.println("Disabling Outputs");
   
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
}


void turnStuffOff(){
  chip0.writePort(0xFFFF);
  if (sysVarArray[1]) chip1.writePort(1,0xFF);    // Pumps or hoppers on chip1 Off
  else chip1.writePort(1,0x00);

  // chip1.writePort(1,pumpState);      // Pumps Off
  
  chip2.writePort(0xFFFF);
  chip3.writePort(1,0x00);    // Off  
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);   // GPIO 5
  chip0.begin();     // This starts SPI and set the chip select
  chip1.begin();     // pin (10) to OUTPUT
  chip2.begin();                             // different from SelfAdmin201.ino
  chip3.begin();                             // different from SelfAdmin201.ino
  for (uint8_t i = 0; i <= 15; i++) {
    chip0.pinMode(i,OUTPUT);             // Set chip0 to OUTPUT
    chip2.pinMode(i,OUTPUT);             // Set chip2 to OUTPUT
  }
  for (uint8_t i = 0; i <= 7; i++)  {
     chip1.pinMode(i,INPUT_PULLUP);          
     chip3.pinMode(i,INPUT_PULLUP);          
  }
  for (uint8_t i = 8; i <= 15; i++) {
     chip1.pinMode(i, OUTPUT);               
     chip3.pinMode(i, OUTPUT);               
  }
  turnStuffOff();  
  portOneValue = chip1.readPort(0);          
  portTwoValue = chip3.readPort(0);          
  // Serial.println(portOneValue,BIN);

  delay(500); 
  init_10_mSec_Timer(); 
  Serial.println("9 Beta");
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

   inputErrors++;
   // Print the error that got us here.
   if (Verbose) Serial.print ("9 ! "+String(millis())+" Port "+String(leverNum));
   
   TStamp tStamp = {10, '!', millis(), 0, 9};
   printQueue.push(&tStamp);

   for (int x = 0; x < 10; x++) {                      // Try ten times
      if (leverNum == 1) _portValue = chip1.readPort(0);
      else _portValue = chip3.readPort(0);
      if (_portValue == 0) Serial.print("! ");         // Still has error
      else {
         recoveredFromError = true;
         inputRecoveries++;
         if (Verbose) Serial.println(" ... recovered after "+String(x+1)+" attempt(s)");
         break;
      }
   }                                                  // If error after 10 tries 
   if (!recoveredFromError) {
      Serial.println();
      resetChips(); 
      if (chip1.readPort(0) != 0 && chip3.readPort(0) != 0) {
        if (Verbose) Serial.println("Recovered from Input Error after chip reset");
           inputRecoveries++;
      }
      else {      
        Serial.println();
        Serial.println("Ending Session Because of Input Errors");
        for (int boxNum = 0; boxNum < 8; boxNum++) 
           boxArray[boxNum].endSession();
      }
   }   
}


void checkLeverOneBits() {
/*
 * boxArray[i].handle_L1_Response() is called when the lever goes to ground.
 */
  
   static byte oldPortOneValue = 255;       
   portOneValue = chip1.readPort(0);
   if (portOneValue == 0) handleInputError(1,portOneValue);     
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
    byte diff = 0;
    static byte oldPortTwoValue = 255;      
    portTwoValue = chip3.readPort(0);
    // **********  compareBits
    for (int i = 7; i > -1; i--) {
      if (bitRead(portTwoValue,i) != (bitRead(oldPortTwoValue,i))) diff++;
    }
    if (diff > diffCriteria) {
      TStamp tStamp = {10, '>', millis(), 0, 9};
      printQueue.push(&tStamp); 
      Serial.println("9 diff_L2_"+String(diff));
      Serial.println("9 L2="+String(portOneValue));
      oldPortTwoValue = portTwoValue;
      // phantomResp++;
    }   
    else if (diff == 1) {
        oldPortTwoValue = portTwoValue;
         // Serial.println (portTwoValue,BIN);
         for (byte i = 0; i < 8; i++) {
             newLeverTwoState[i] = bitRead(portTwoValue,i);
             if (newLeverTwoState[i] != lastLeverTwoState[i]) {          
                  lastLeverTwoState[i] = newLeverTwoState[i]; 
                  boxArray[i].handle_L2_Response(newLeverTwoState[i]);
             }
         }
    }    
}           

/*
void decodeSysVars(byte varCode) {
  byte mask;
  byte result; 
  for (byte i = 0; i < 8; i++) {    
     mask = pow(2,i);           // mask (eg. 00001000)
     // Serial.println("mask = "+String(mask));
     result = varCode & mask;    // Uses AND and mask to determine whether to set bit
     // Serial.println("result = "+String(result));
     if ( result > 0) sysVarArray[i] = true;
     else sysVarArray[i] = false;
  }
  for (byte i = 0; i < 8; i++) {
    Serial.print(sysVarArray[i]);
    }
    Serial.println("9 varCode="+String(varCode));
  // leverTwoExists = sysVarArray[2]; 
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
     if (stringCode == "chip0") chip0.digitalWrite(num1,num2); 
     else if (stringCode == "G")     boxArray[num1].startSession();
     else if (stringCode == "Q")     boxArray[num1].endSession();
     else if (stringCode == "L1")    boxArray[num1].handle_L1_Response(); 
     else if (stringCode == "P")     boxArray[num1].switchTimedPump(On);
     // else if (stringCode == "p")     boxArray[num1].switchTimedPump(Off);
     else if (stringCode == "PROTOCOL") boxArray[num1].setProtocolNum(num2);
     else if (stringCode == "PARAM") boxArray[num1].setParamNum(num2);
     else if (stringCode == "TIME")  boxArray[num1].setBlockDuration(num2);  
     else if (stringCode == "IBI")   boxArray[num1].setIBIDuration(num2);         
     else if (stringCode == "PUMP")  boxArray[num1].setPumpDuration(num2); 
     else if (stringCode == "R")     boxArray[num1].reportParameters();
     else if (stringCode == "=")     boxArray[num1].moveLeverOne(Extend);   // extend lever1
     else if (stringCode == ".")     boxArray[num1].moveLeverOne(Retract);    // retract lever1
     else if (stringCode == "~")     boxArray[num1].moveLeverTwo(Extend);  
     else if (stringCode == ",")     boxArray[num1].moveLeverTwo(Retract);
     else if (stringCode == "s")     boxArray[num1].switchStim1(Off);
     else if (stringCode == "S")     boxArray[num1].switchStim1(On);
     else if (stringCode == "c")     boxArray[num1].switchStim2(Off);
     else if (stringCode == "C")     boxArray[num1].switchStim2(On);
     else if (stringCode == "V")     Serial.println("9 Ver=301.00");
     else if (stringCode == "D")     reportDiagnostics(); 
     /*
     // debug stuff 
     else if (stringCode == "i")     timeUSB();
     else if (stringCode == "E")     echoInput = !echoInput;
     */
     inputString = "";
   }
}

int freeRam () {
  char stack_dummy = 0;
  return &stack_dummy - sbrk(0);
}

void reportDiagnostics() {
   Serial.println("9 maxDelta="+String(maxDelta));
   maxDelta = 0;
   Serial.println("9 maxQueueRecs="+String(maxQueueRecs));
   Serial.println("9 freeRam="+String(freeRam()));
   Serial.println("9 inputErrors="+String(inputErrors));
   Serial.println("9 inputRecoveries="+String(inputRecoveries));
   Serial.println("9 outputErrors="+String(outputErrors));
   Serial.println("9 outputRecoveries="+String(outputRecoveries));
   portTwoValue = chip3.readPort(0);
   Serial.println("9 portTwoValue="+String(portTwoValue));
   Serial.print("9 ");
   for (byte i = 0; i < 8; i++) {
      if (newLeverTwoState[i] != boxArray[i].pumpOn) Serial.print(".");
      else Serial.print("!");
   }
   Serial.println();    
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
   if (tickCounts == 100) {
       digitalWrite(ledPin, !digitalRead(ledPin));
       tickCounts = 0;
   }
   for (uint8_t i = 0; i < 8; i++) boxArray[i].tick();
   getInputString();
   checkLeverOneBits();
   checkLeverTwoBits(); 
   sendOneTimeStamp();
   delta = micros() - micro1;
   if (delta > maxDelta) maxDelta = delta;   
}

void loop() {
   if (tickFlag) {
      tickFlag = false;
      tick();
   }
}
