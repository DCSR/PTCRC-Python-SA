/*  
 *   <SYSVARS num> and DecodeSySVars(num) sets eight sysVars
 *   
 *   To Do:
 *   SwitchRewardDevice(On) etc. 
 *    Time stamp should reflect Pump (P,p) or Hopper (H,h)
 *   
 *   Incorporate Device??
 *   If not: 
 *   -  rename switchPump to switchDevice
 *   -  use rewardType to determine port
 *   -  rename _pumpTime, _pumpDuration etc. to _deviceTime
 *   -  also _pumpOn
 *   
 *   Change labels for sysVars0-2 in Python
 *   
 *   Done on April 7th
 *   Defined an array of eight boolena variables.
 *   twoLever removed
 *   debugBoolVarList removed
 *   setRewardType removed
 *   rewardType defined globally  Drug = 0, Food = 1
 *   sysVarArray[1] substituted for pumpOnHigh
 *   
 *   Document how the timing is done and relate it to the PowerPoint slide.
 *    Reinforce() changes _timeOut = true which is checked in tick() each 10 mSec.
 *   - use only one _rewardTimer
 *   - _timeOut can be different and longer than pump
 *   
 *   Test: 
 *   Programm was changed from decrementing a timer value to incrementing.
 *   Why? It seems more intuitive to count down.
 *   if (_pumpTime > 0) {
 *      _pumpTime--;
 *      if (_pumpTime == 0) switchOff
 *   }
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
 *   void Lever::switchPump(boolean state) {
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
 *   rewardType is now a global variable that determines Drug or Food
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
 *   Eventually change the Pump and LED to devices.
 *   
 *   Changes: 
 *   _pumpTime++ until equals pumpDuration
 *   _blockTime++ until equals _blockDuration
 *   _IBILength changed to _IBIDuration
 *   _pumpTimer changed to _pumpTime
 *   states _boxState = PRESTART; 
 *   
 *   _blockTime was used for IBI as well - created _IBITime
 *   
 *   Previous (commented out) checkLeverOneBits() requires input to go LOW for two ticks 
 *   in order to trigger a response.
 *   In theory, this should filter out a brief spike (noise?) from triggering a response. 
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
 * Chip3, Port1:  8 spare DIOs
 * 
 */

#include <cppQueue.h>
#include <SPI.h>        // Arduino Library SPI.h
#include "MCP23S17.h"   
#include "Device.h"

// Device pump1(1);

const uint8_t chipSelect = 10;  // All four chips use the same SPI chipSelect
MCP23S17 chip0(chipSelect, 0);  // Instantiate 16 pin Port Expander chip at address 0
MCP23S17 chip1(chipSelect, 1);  
MCP23S17 chip2(chipSelect, 2);    
MCP23S17 chip3(chipSelect, 3); 

boolean sysVarArray[8] = {false,false,false,false,false,false,false,false};
// sysVarArray[0] used for reward type; 0 = Drug, 1 = Food
// sysVarArray[1] used for logic type: 0 = 5VDC switches On, 1 = GND switches On


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
byte portOneValue, portTwoValue;
String instruction;

boolean lastLeverOneState[8] = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
boolean newLeverOneState[8] = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
boolean newResponse[8] = {false,false,false,false,false,false,false,false};
byte ticks[8] = {0,0,0,0,0,0,0,0};
int lastLeverTwoState[8] = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
int newLeverTwoState[8] = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};

volatile boolean tickFlag = false;

// program housekeeping varaibles
String inputString;
boolean sessionRunning = false;
boolean echoInput = false;
unsigned long maxDelta = 0;
byte maxQueueRecs = 0;
int phantomResp = 0;
byte diffCriteria = 1;

// ***************************  Box Class *************************************

enum  states { PRESTART, BLOCK, IBI, FINISHED };

class Lever {
  public:
    Lever(int boxNum);
    void tick();
    void startSession();
    void endSession();
    void setProtocolNum(int protocalNum);  // set with lever1._protocolNum = protocolNum
    void setPumpDuration(int pumpDuration);
    void setParamNum(int paramNum);
    void setBlockDuration(int blockDuration); 
    void handleResponse();
    void switchPump(boolean state);
    void switchStim1(boolean state);
    void switchStim2(boolean state);
    void moveLever(int state);
    int _boxNum;
    
  // private:
    int _tickCounts = 0;
    void startBlock();
    void endBlock();
    void startTrial();
    void endTrial();
    void startIBI();
    void endIBI();
    void reinforce();
    void startTimeOut();
    void endTimeOut();

    // void moveLever2(int state);
 
    boolean _timeOut = false;                     
    boolean _schedPR = false;
    boolean _schedTH = false;
    // defaults to a 6h FR1 session 
    int _protocolNum = 1;  // ['0: Do not run', '1: FR', '2: FR x 20', '3: FR x 40', '4: PR', '5: TH', '6: IntA: 5-25']
    int _paramNum = 1;
    int _PRstepNum = 1;   
    states _boxState = PRESTART;
    // Block
    unsigned long _blockDuration = 21600;  // 60 * 60 * 6 = 21600 seconds = 6 hrs;
    unsigned long _blockTime = 0;    // unsigned int would only allow 65,535 seconds = 18.2 hours
    unsigned int _blockNumber = 0;
    unsigned int _maxBlockNumber = 1;  
    // Trial 
    unsigned int _responseCriterion = 1;  // default to FR1
    unsigned int _trialNumber = 0;
    unsigned int _maxTrialNumber = 999;
    unsigned int _trialResponses = 0;
    // Reinforce 
    int _pumpDuration = 400;    // 400 x 10 mSec = 4,000 mSec = 4 seconds;
    int _pumpTime = 0;    
    int _THPumpTimeArray[13] = {316, 316, 200, 126, 79, 50, 32, 20, 13, 8, 5, 3, 2};
    // int _THPumpTimeArray[13] = {100, 100, 50, 40, 30, 20, 10, 9, 8, 7, 5, 3, 2};    
    int _timeOutTime = 0;
    int _timeOutDuration = 400;  // default to 4 sec    
    // IBI     
    unsigned long _IBIDuration = 0;
    unsigned long _IBITime = 0;    
    unsigned long _startTime = 0;
};

// ************* Lever Class Procedures *************************************

Lever::Lever(int boxNum) {
  _boxNum = boxNum;
}

void Lever::tick(){       
    if (_timeOut) {
      _pumpTime++;
      if (_pumpTime == _pumpDuration) switchPump(Off);  // change to pump.update() 
      _timeOutTime++;
      if (_timeOutTime == _timeOutDuration) endTimeOut();     
    }
    _tickCounts++; 
    if (_tickCounts == 100)    {         // do this every second
       _tickCounts = 0;
       //switch (_boxState) {  // PRESTART, BLOCK, IBI, FINISHED 
       //   case BLOCK:
       if (_boxState == BLOCK) {
            _blockTime++;
            TStamp tStamp = {_boxNum, '*', _blockTime, 0, 9};
            printQueue.push(&tStamp);
            if (_blockTime >+ _blockDuration) endBlock();
            }
        else if (_boxState == IBI) {
            _IBITime++;
            TStamp tStamp = {_boxNum, '*', _IBITime, 0, 9};
            printQueue.push(&tStamp);
            if (_IBITime >= _IBIDuration) endIBI(); 
            }      
       }  
}

void Lever::startSession() { 
  // Set Protocol defaults here
  // Python protocol list:  
  // ['0: Do not run', '1: FR', '2: FR x 20', '3: FR x 40', 
  // '4: PR', '5: TH', '6: IntA: 5-25', '7: Debug']

   _timeOutDuration = _pumpDuration;     // except in protocol 2

  if (_protocolNum == 0) endSession();
  else  {  
      if (_protocolNum == 1) {           // FR(N)
        _schedPR = false;
        _schedTH = false;
        _maxTrialNumber = 999;           
        _responseCriterion = _paramNum;
        /*
        _responseCriterion = 2;         // for debug
        _blockDuration = 60;
        _IBIDuration = 10;                  // no IBI
        _maxBlockNumber = 2;
        Serial.println(String(_boxNum)+" started");
        */
      }
      else if (_protocolNum == 2) {      // FR1 x 20
        _schedPR = false;
        _schedTH = false;
        _maxTrialNumber = 20;
        _responseCriterion = 1;
        _timeOutDuration = 2000;         // 10 mSec x 2000 = 20 sec
      }
      else if (_protocolNum == 3) {      // FR x N
        _schedPR = false;
        _schedTH = false;
        _maxTrialNumber = _paramNum;
        _responseCriterion = 1;
      }
      else if (_protocolNum == 4) {      // PR(N)
        _schedPR = true;
        _PRstepNum  = _paramNum;
        _schedTH = false;
      }   
      else if (_protocolNum == 5) {      // TH
        _schedPR = false;
        _schedTH = true;
        _maxTrialNumber = 4;
        _blockDuration = 21600;          // 60 * 60 * 6 = 21600 seconds = 6 hrs; This changes in Block 2
        _IBIDuration = 0;                  // no IBI
        _maxBlockNumber = 13;            // 13 blocks
      }
      else if (_protocolNum == 6) {      // IntA 5-25 6h
        _schedPR = false;
        _schedTH = false;
        _blockDuration = 300;            // 60 seconds * 5 min
        _IBIDuration = 1500;               // 25 * 60 sec = 1500 seconds
        _maxBlockNumber = 12;            // 12 blocks
      }
      /*
      else if (_protocolNum == 7) {      // Debug
        _schedPR = false;                // Sets up initially as an FR1 
        _schedTH = false;
        _responseCriterion = 1;
        _maxTrialNumber = 999;
        _cycles = _paramNum;
        _pumpOnTime = _pumpDuration;
        _blockDuration = 21600;          // default to 6hr
        _timeOutDuration = ((_cycles+1)*(_pumpOnTime + _pumpOffTime));
        _cyclePump = false;             // This is the thing that controls the cycle in tick()  
      } 
      */  
      _startTime = millis();
      _blockNumber = 0;  
      _pumpTime = 0; 
      _timeOutTime = 0;
      // sessionRunning = true;     
      TStamp tStamp = {_boxNum, 'G', millis() - _startTime, 0, 9}; 
      printQueue.push(&tStamp);
      startBlock();
  }
  Serial.println("9 "+String(_boxNum)+"_"+String(_responseCriterion)+"_"+String(_protocolNum)); 
}

void Lever::endSession () {   
    // endTrial(); the only thing this did was retract the lever, but see next line. 
    // _cyclePump = false;
    moveLever(Retract);         // was moveLever1
    // moveLever2(Retract);
    switchStim1(Off);
    switchPump(Off);
    _pumpTime = 0;
    _timeOutTime = 0;
    _boxState = FINISHED;    
    TStamp tStamp = {_boxNum, 'E', millis() - _startTime, 0, 9};
    printQueue.push(&tStamp);
}

void Lever::setProtocolNum(int protocolNum) {
  _protocolNum = protocolNum;
}

void Lever::setPumpDuration(int pumpDuration) {
  _pumpDuration = pumpDuration;
}

void Lever::setParamNum(int paramNum) {
  _paramNum = paramNum;
  // Serial.println("Setting Lever _paramNum to "+String(_paramNum));
}

void Lever::setBlockDuration(int blockDuration) {
  _blockDuration = blockDuration;
}

void Lever::handleResponse() { 
  if (_timeOut == false) {  
       TStamp tStamp = {_boxNum, 'L', millis() - _startTime, 0, 9};
       printQueue.push(&tStamp);
       _trialResponses++;
       if (_trialResponses >= _responseCriterion) {
               endTrial();
               reinforce();
               startTimeOut();
       }
    }
}


void Lever::startBlock() {
  _blockTime = 0;
  _trialNumber = 0;
  _blockNumber++;
  _boxState = BLOCK;
  TStamp tStamp = {_boxNum, 'B', millis() - _startTime, 0, 9};
  printQueue.push(&tStamp);  
  if (_schedTH == true){                                    // TH
      _pumpDuration = _THPumpTimeArray[_blockNumber - 1];    // zero indexed array; block 1 = index 0
      _timeOutDuration = _pumpDuration;
  }
  startTrial();
}

void Lever::endBlock() {   
   TStamp tStamp = {_boxNum, 'b', millis() - _startTime, 0, 9};
   printQueue.push(&tStamp);
   if (_blockNumber < _maxBlockNumber) startIBI();
   else endSession();
}

void Lever::startTrial() {   
   _trialResponses = 0;
   _trialNumber++;
   if (_schedPR == true) _responseCriterion = round((5 * exp(0.2 * _PRstepNum)) - 5);    // Sched = PR
   _PRstepNum++;
   moveLever(Extend);     // extend lever
   _timeOut = false;      
   TStamp tStamp = {_boxNum, 'T', millis() - _startTime, 0, 9};
   printQueue.push(&tStamp);
}

void Lever::endTrial() {
   if (_protocolNum != 5) moveLever(Retract);       // if not TH then retract lever
}

void Lever::startIBI() {  
   if (_IBIDuration == 0) startBlock();
   else
   {   moveLever(Retract);
       _IBITime = 0;    // tick will handle when to end IBI 
       _boxState = IBI;      // IBI
   }    
}

void Lever::endIBI() {
   startBlock();
}

void Lever::reinforce() { 
    _pumpTime = 0;
    switchPump(On);
}

void Lever::startTimeOut() {
    switchStim1(On); 
    _timeOutTime = 0;       // _timeOutTimer counts down with each tick
    _timeOut = true;                  //timeOut    
    TStamp tStamp = {_boxNum, 't', millis() - _startTime, 0, 9};
    printQueue.push(&tStamp);
}

void Lever::endTimeOut() {
    _timeOut = false;  
    switchStim1(Off);
    if (_trialNumber < _maxTrialNumber) startTrial(); 
    else {
      if (_schedTH == true) {
        _maxTrialNumber = 999;    // _maxTrialNumber is 4 for block 1 and 999 thereafter.
        _blockDuration = 600;     // Block duration in seconds; 60 = 1 min; 600 = 10 min
      }
      endBlock();
    }  
}

void Lever::switchPump(boolean state) {

    // rename switchPump to switchDevice and use rewardType to determine port   
    // boxNum 0..7 maps to pin 0..7 on chip1 or chip3 
    // Normally: On (true) switches the bit to HIGH
    //       and Off (false) switches the bit to LOW
    // BUT if sysVarArray[1] == true -> then the reverse happens
   
    boolean level;
    if (sysVarArray[1]) level = !state;
    else level = state; 

    // Pumps are usually wired to Port1 and hoppers to Port3
    if (sysVarArray[0] == 0) {
      chip1.digitalWrite(_boxNum+8,level);
      Serial.println("port1 ");
    }
    else {
      chip3.digitalWrite(_boxNum+8,level);
      Serial.println("port3 ");
    }
       
    if (state) {              // pumpOn == true 
      
          TStamp tStamp = {_boxNum, 'P', millis() - _startTime, 1, 2};
          printQueue.push(&tStamp);
    }
    else {
      
          TStamp tStamp = {_boxNum, 'p', millis() - _startTime, 0, 2};
          printQueue.push(&tStamp);
    }
    // The Pump CheckBox is index 2 
}

void Lever::switchStim1(boolean state) {
    boolean level;
    level = !state;                        // On = true -> level goes low 
    chip0.digitalWrite(_boxNum+8,level);   // boxNum 0..7 maps to pin 8..15 on chip0
    // HIGH = OFF
    if (state) {
          TStamp tStamp = {_boxNum, 'S', millis() - _startTime, 1, 3};
          printQueue.push(&tStamp);
    }
    else {
          TStamp tStamp = {_boxNum, 's', millis() - _startTime, 0, 3};
          printQueue.push(&tStamp);
    }
    // StimCheckBox is index 3     
}

void Lever::switchStim2(boolean state) {
    boolean level;
    level = !state;                        // On = true -> level goes low    
    chip2.digitalWrite(_boxNum+8,level);   // boxNum 0..7  maps to pin 8..15 on chip2
    // HIGH = OFF
    if (state) {
          TStamp tStamp = {_boxNum, 'C', millis() - _startTime, 1, 4};
          printQueue.push(&tStamp);
    }
    else {
          TStamp tStamp = {_boxNum, 'c', millis() - _startTime, 0, 4};
          printQueue.push(&tStamp);
    }
    // StimCheckBox is index 4     
}

void Lever::moveLever(int state) {          // boxNum 0..7  maps to pin 0..7 on chip0
    chip0.digitalWrite(_boxNum,state);
    // HIGH = Retract; LOW = extend
    if (state) {
           TStamp tStamp = {_boxNum, '.', millis() - _startTime, 0, 0};
           printQueue.push(&tStamp); 
    }
    else {
          TStamp tStamp = {_boxNum, '=', millis() - _startTime, 1, 0};
          printQueue.push(&tStamp);
    }
    // Lever1 (active) CheckBox is index 0 
}

/*
void Box::moveLever2(int state) {          // boxNum 0..7  maps to pin 0..7 on chip2
    chip2.digitalWrite(_boxNum,state);
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
*/


// **************  Box Class Procedures *************************************

class Box  {
  public:
    Lever lever1;  
    Box(int boxNum) : lever1(boxNum) {
        _boxNum = boxNum; 
    }
    void startSession();
    void endSession();
    // void cyclePump();
    void tick();
    void handle_L1_Response();
    void setProtocolNum(int protocalNum);
    void setPumpDuration(int pumpDuration);
    void setParamNum(int paramNum);
    void setBlockDuration(int blockDuration);    
    void reportParameters();
    void getBlockTime();
    
    //************
  private:
    int _boxTicks = 0;  // used for flashing LED and printTime
    int _boxNum;
    // used in debug protocol 7 
    boolean _cyclePump = false;
    int _cycleCount = 0;
    int _cycles;
    int _pumpOnTime;          // set in StartSession()
    int _pumpOffTime = 20;    // default
    int _pumpOnTicker = 0;
    int _pumpOffTicker = 0;    
};

void Box::startSession() {
  lever1.startSession();
}

void Box::endSession() {
  lever1.endSession();
}

/*
void Box::cyclePump(){
    if (_pumpOnTicker > 0) {
       _pumpOnTicker--;
       if (_pumpOnTicker == 0) {
           if (sysVarArray{1]) chip1.digitalWrite(_boxNum+8,LOW);
           else chip1.digitalWrite(_boxNum+8,HIGH);
          _pumpOffTicker = _pumpOffTime;
          _cycleCount++;
          if (_cycleCount == _cycles) _cyclePump = false;
       }
    }
    else if (_pumpOffTicker > 0) {
        _pumpOffTicker--;
        if (_pumpOffTicker == 0) {
           if (sysVarArray[1]) chip1.digitalWrite(_boxNum+8,HIGH);
           else chip1.digitalWrite(_boxNum+8,LOW);
          _pumpOnTicker = _pumpOnTime;
        }
    }
}
*/

void Box::tick() { // do stuff every 10 mSec 
  _boxTicks++;
  if (_boxTicks == 100) {
    _boxTicks = 0;
    // Probabaly no need for boxTicks
  }
  lever1.tick();
}

void Box::handle_L1_Response() { 
  lever1.handleResponse();
}

/*
void Box::handle_L2_Response() {           // Inactive lever press
  TStamp tStamp = {_boxNum, 'J', millis() - _startTime, 0, 9};
  printQueue.push(&tStamp);
}
*/

void Box::setProtocolNum(int protocolNum) {
  // _protocolNum = protocolNum;
  lever1.setProtocolNum(protocolNum);
}

void Box::setPumpDuration(int pumpDuration) {
  // _pumpDuration = pumpDuration;
  lever1.setPumpDuration(pumpDuration);
}

void Box::setParamNum(int paramNum) {
  // _paramNum = paramNum;
  // Serial.println("Setting Box paramNum to "+String(paramNum));
  lever1.setParamNum(paramNum);
}

void Box::setBlockDuration(int blockDuration) {
  // _blockDuration = blockDuration;
  lever1.setBlockDuration(blockDuration);
}

void Box::reportParameters() { 
  Serial.print("9 "+String(lever1._boxNum)+":"+String(lever1._protocolNum)+":");
  Serial.print(String(lever1._responseCriterion)+":"+String(lever1._blockDuration)+":");
  Serial.print(String(lever1._pumpDuration)+":"+String(lever1._timeOutDuration));
  Serial.println("-"+String(lever1._maxTrialNumber));
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

void turnStuffOff(){
  chip0.writePort(0xFFFF);
  if (sysVarArray[1]) chip1.writePort(1,0xFF);    // Pumps or hoppers on chip1 Off
  else chip1.writePort(1,0x00);
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
     chip1.pinMode(i,INPUT_PULLUP);          // Ver 200.04
     chip3.pinMode(i,INPUT_PULLUP);          // Ver 200.04
  }
  for (uint8_t i = 8; i <= 15; i++) {
     chip1.pinMode(i, OUTPUT);               // Ver 200.04
     chip3.pinMode(i, OUTPUT);               // Ver 200.04
  }
  turnStuffOff();  
  portOneValue = chip1.readPort(0);          // Ver 200.04
  portTwoValue = chip3.readPort(0);          // Ver 200.04
  // Serial.println(portOneValue,BIN);

  delay(500); 
  init_10_mSec_Timer(); 
  Serial.println("9 StateBeta");
}

void checkLeverOne() {
    static byte oldPortOneValue = 255;       
    portOneValue = chip1.readPort(0);                
    if (portOneValue != oldPortOneValue) {
         oldPortOneValue = portOneValue;
         for (byte i = 0; i < 8; i++) {
             newLeverOneState[i] = bitRead(portOneValue,i);
             if (newLeverOneState[i] != lastLeverOneState[i]) {          
                  lastLeverOneState[i] = newLeverOneState[i]; 
                  if (newLeverOneState[i] == 0) {
                     boxArray[i].handle_L1_Response();
                     String tempStr = "9 L1_Response:pin_"+String(i); 
                     Serial.println(tempStr);
                  }
             }
         }    
    }           
}


void checkLeverOneBits() {
    byte diff = 0;
    static byte oldPortOneValue = 255;       
    portOneValue = chip1.readPort(0);
    // **********  compareBits
    for (int i = 7; i > -1; i--) {
      if (bitRead(portOneValue,i) != (bitRead(oldPortOneValue,i))) diff++;
    }  
    if (diff > diffCriteria) {
      Serial.println("9 diff_"+String(diff));
      Serial.println("9 "+String(portOneValue));
      oldPortOneValue = portOneValue;
      phantomResp++;
    }
    else if (diff == 1) {
         oldPortOneValue = portOneValue;
         // Serial.println (portOneValue,BIN);
         for (byte i = 0; i < 8; i++) {
             newLeverOneState[i] = bitRead(portOneValue,i);
             if (newLeverOneState[i] != lastLeverOneState[i]) {          
                  lastLeverOneState[i] = newLeverOneState[i]; 
                  if (newLeverOneState[i] == 0) {
                     boxArray[i].handle_L1_Response();
                     String tempStr = "9 L1_Response:pin_"+String(i); 
                     Serial.println(tempStr);
                  }
             }
         }    
    }           
}

/*
void checkLeverTwo() {
    static byte oldPortTwoValue = 255;      
    portTwoValue = chip3.readPort(0);
    if(portTwoValue != oldPortTwoValue) {
        oldPortTwoValue = portTwoValue;
         // Serial.println (portTwoValue,BIN);
         for (byte i = 0; i < 8; i++) {
             newLeverTwoState[i] = bitRead(portTwoValue,i);
             if (newLeverTwoState[i] != lastLeverTwoState[i]) {          
                  lastLeverTwoState[i] = newLeverTwoState[i]; 
                  if (newLeverTwoState[i] == 0) {
                     boxArray[i].handle_L2_Response();
                     String tempStr = "9 L2_Response:pin_"+String(i); 
                     Serial.println(tempStr);
                  }
             }
         }    
    }           
}
*/

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
    Serial.println("9 varCode = "+String(varCode));
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
     else if (stringCode == "G")     boxArray[num1].startSession();
     else if (stringCode == "Q")     boxArray[num1].endSession();
     else if (stringCode == "L1")    boxArray[num1].lever1.handleResponse(); 
     else if (stringCode == "P")     boxArray[num1].lever1.switchPump(On);
     else if (stringCode == "p")     boxArray[num1].lever1.switchPump(Off);
     else if (stringCode == "SCHED") boxArray[num1].lever1.setProtocolNum(num2);
     else if (stringCode == "PUMP")  boxArray[num1].lever1.setPumpDuration(num2); 
     else if (stringCode == "RATIO") boxArray[num1].setParamNum(num2);
     else if (stringCode == "TIME")  boxArray[num1].lever1.setBlockDuration(num2);
     else if (stringCode == "R")     boxArray[num1].reportParameters();
     else if (stringCode == "=")     boxArray[num1].lever1.moveLever(Extend);   // extend lever1
     else if (stringCode == "-")     boxArray[num1].lever1.moveLever(Retract);    // retract lever1
     else if (stringCode == "s")     boxArray[num1].lever1.switchStim1(Off);
     else if (stringCode == "S")     boxArray[num1].lever1.switchStim1(On);
     else if (stringCode == "c")     boxArray[num1].lever1.switchStim2(Off);
     else if (stringCode == "C")     boxArray[num1].lever1.switchStim2(On);
     else if (stringCode == "V")     Serial.println("9 Beta");
     else if (stringCode == "D")     reportDiagnostics(); 
     else if (stringCode == "SYSVARS") decodeSysVars(num1); 
     else if (stringCode == "Logic") {
      Serial.println(sysVarArray[num1]);
      Serial.println(sysVarArray[0]);
     }
     /*
     // debug stuff 
     else if (stringCode == "L1")    boxArray[num1].handle_L1_Response();
     else if (stringCode == "off")   turnStuffOff();
     else if (stringCode == "i")     timeUSB();
     else if (stringCode == "E")     echoInput = !echoInput;
     else if (stringCode == "B")     boxArray[num1].getBlockTime();
     ****** Deprecated - check Python for codes being sent
     // else if (stringCode == "1")     boxArray[0].handleResponse();
     // else if (stringCode == "2")     boxArray[1].handleResponse();
     // else if (stringCode == "~")     boxArray[num1].moveLever2(Extend);   // extend lever2  
     // else if (stringCode == ",")     boxArray[num1].moveLever2(Retract);    // retract lever2
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
   Serial.println("9 phantomResp="+String(phantomResp));
   phantomResp = 0;   
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
