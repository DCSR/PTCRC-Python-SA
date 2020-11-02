
/*
 * November 1
 * 
 * Priority:
 *    1. Detect and report... OK but needs Steven to break it.
 *    2. Try to recover with a function
 *    3. Integrate recovery of inputPorts with tick function so that timing of pumps etc
 *    can be maintaied.
 *    4. If all else fails, switch output ports to inputs which would (presumably) turn off pumps etc.
 *    
 *    showPorts() - Show all ports. No error checking 
 *    checkOutputPorts() - Compare registers. Print Errors, No recovery attempted.
 *    reportOutputErrors() - Show the ports with errors.
 *    handleOutputError() - retry a few times then abort. 
 *    
 * 
 * Tick:
 *    checkInputPort1() -> handleInputError(1)
      checkInputPort2() -> handleInputError(2)
      checkOutputPorts() -> handleOutputError(printStuff)
        check and report errors
        and shut stuff down if necessary

        Time how long that would take each cycle - assuming no errors.

        Would it be possible to get a false positve error due to I2C and speed of the port expander chip?
        
 * 
 * 
 * 
 * 
 * 
 * 
 * everythingOff() deleted
 * 
 * checkInputPort2() 
 *    - No longer detects diffs
 *    - checks outputs
 *    - timing = 
 *    
 * 
 * 
 * 
 * To Do: showBits(c) might be replaced with Serial.println(c,BIN);
 * 
 * This tests proof of principle for coupling input and output ports for Hold Down. 
 * 
 * It is adapted from a sketch in Arduino forum:
 * http://arduino.stackexchange.com/questions/28488/arduino-mcp23s17-i-o-expander-spi
 * Uses library from: github.com/MajenkoLibraries/MCP23S17
 * 
 * Note that there are new versions in the MajenkoLibraries
 * 
 * chip0.writePort(0xFFFF);        sets all 16 pins HIGH
 * chip1.writePort(1,0xFF);        sets all 8 pins on Port 1 HIGH
 * chip0.digitalWrite(pinNum,1);   sets pin HIGH
 * 
 */

#include <cppQueue.h>
#include <SPI.h>        // Arduino Library SPI.h
#include "MCP23S17.h"   // Majenko MCP23S17 library

const uint8_t chipSelect = 10;  
MCP23S17 chip0(chipSelect, 0); // L1 retract map to pins 0..7; L1 LEDs map to pins 8..15 
MCP23S17 chip1(chipSelect, 1); // L1 inputs map to pins 0..7; Pumps map to pins 8..15
MCP23S17 chip2(chipSelect, 2); // L2 retract map to pins 0..7; LEDs map to pins 8..15
MCP23S17 chip3(chipSelect, 3); // L2 inputs map to pins 0..7; AUX output map to pins 8..15

// Input Port Values
byte portOneValue = 255, portTwoValue = 255, oldPortTwoValue = 255; 

// Output Ports Values
byte L1_Position = 0xFF;
byte L1_LED_State = 0xFF;      
byte pumpState = 0x00; 
byte pumpStateL1 = 0x00;
byte pumpStateL2 = 0x00;
byte L2_Position = 0xFF;
byte L2_LED_State = 0xFF; 

long micro1, micro2;
long minDelta = 1000;
long maxDelta = 0;
byte maxQueueRecs = 0;
String instruction;
boolean sessionRunning = false;
boolean sendTimeStamp = false;
long startTime;
int inputErrors = 0;
int inputRecoveries = 0;
int outputErrors = 0;
int outputRecoveries = 0;
int L1_responses = 0;
int L2_responses = 0;  

typedef struct tagTStamp {
   // tagTStamp is a structure identifier (or tag). It is not necessary 
   // to specify the tag except when you need a pointer to the struct 
   byte boxNum;
   char code;
   uint32_t mSecTime;
   byte state;
   byte index;
} TStamp;

Queue printQueue(sizeof(TStamp), 60, FIFO); // Instantiate printQueue, First In First Out
  // Note: maximum in the queue was 40 when quitting all eight boxes at once (w/o a delay)

// ********** 10 mSec Interrupt Timer ***************** 

volatile boolean tickFlag = false;
// *************************** Timer stuff *********************************************
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
//************************* End Timer stuff ***********************************************


void TC4_Handler()                                // Interrupt Service Routine (ISR) for timer TC4
{     
  // Check for overflow (OVF) interrupt
  if (TC4->COUNT16.INTFLAG.bit.OVF && TC4->COUNT16.INTENSET.bit.OVF)  {
    tickFlag = true;
    REG_TC4_INTFLAG = TC_INTFLAG_OVF;             // Clear the OVF interrupt flag
  }
}

// ******************************************************

void configureChips() {
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
}

boolean resetAndRecover() {
  // Returns true if reset and no output errors detected
  boolean recovered = false;
  chip0.begin();
  chip1.begin();
  chip2.begin();
  chip3.begin();
  configureChips();
  chip0.writePort(0,L1_Position);  
  chip0.writePort(1,L1_LED_State);         
  chip2.writePort(0,L2_Position);
  chip2.writePort(1,L2_LED_State); 
  if (!checkOutputPorts()) recovered = true; // no error found.
  return recovered;
}

void showMenu () {
  Serial.println ("HD_Demo.ino.");
  Serial.println ("<m> - Show this Menu");
  Serial.println ("<G> - Start HD Session - Feather LED should blink");
  Serial.println ("<Q> - End HD Session"); 
  Serial.println ("<P num> - Switch Pump On");
  Serial.println ("<p num> - Switch Pump Off");
  Serial.println ("<T> - send timeStamps");
  Serial.println ("<t> - suppress timeStamps");   
  Serial.println ("<H> - handleInputError(2,0x00)");
  Serial.println ("<D> - showDiagnosticData()");
  Serial.println ("<R> - showPorts()");
  Serial.println ("<O> - checkOutputPorts()");
  Serial.println ("<X> - resetAndRecover()");
}

void disableOutputs() {
  Serial.println("Disabling Outputs");
  for (uint8_t i = 0; i <= 15; i++) {
      chip0.pinMode(i,INPUT);             // Set chip0 to OUTPUT
      chip2.pinMode(i,INPUT);             // Set chip2 to OUTPUT
  }
  for (uint8_t i = 8; i <= 15; i++) {
     chip1.pinMode(i, INPUT);               
     chip3.pinMode(i, INPUT);               
  }
}

void startSession() {
  configureChips();
  inputErrors = 0;
  inputRecoveries = 0;
  outputErrors = 0;
  outputRecoveries = 0;
  L1_responses = 0;
  L2_responses = 0; 
  startTime = millis();
  Serial.println("Starting Session");
  L1_Position = 0x00;              // Extend L1
  chip0.writePort(0,L1_Position);  
  L1_LED_State = 0xFF;             // L1 LED Off
  chip0.writePort(1,L1_LED_State);         
  L2_Position = 0x00;              // Extend L2
  chip2.writePort(0,L2_Position);
  L2_LED_State = 0xFF;             // L2 LED Off
  chip2.writePort(1,L2_LED_State); 
  digitalWrite(LED_BUILTIN, HIGH);    

  if (checkOutputPorts()) {
    Serial.println("*********** Failed on OutPutError **************");
    reportOutputErrors();
    Serial.println("*** Try Restarting Feather and Circuit Board *** ");
    endSession();
  }
  else {
    Serial.println("... Started");
    sessionRunning = true;
  }
}

void endSession() {
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
  digitalWrite(LED_BUILTIN, LOW);   
  sessionRunning = false;
  Serial.println("Session Ended");
  disableOutputs();  
  showDiagnosticData(); 
}

String binString (byte c) {
    String binStr = "";
    for (int bits = 7; bits > -1; bits--) {
    // Compare bits 7-0 in byte
    if (c & (1 << bits)) {
      binStr = binStr + "1";
    }
    else {
      binStr = binStr + "0";
    }
  }
  return binStr;
}

void showDiagnosticData() {
  Serial.println ("***** Diagnostic Data *****");
  Serial.print ("Min and Max Deltas in microSec: "); 
  Serial.println (String(minDelta)+" "+String(maxDelta));
  Serial.println ("inputErrors / inputRecoveries = "+String(inputErrors)+" "+String(inputRecoveries));
  Serial.println ("outputErrors / outputRecoveries = "+String(outputErrors)+" "+String(inputRecoveries)); 
  Serial.println ("L1 responses = "+String(L1_responses));
  Serial.println ("L2 responses = "+String(L2_responses));
  minDelta = 1000;
  maxDelta = 0;
}

void showPorts() {
  // byte L1_Position_PortValue, L1_LED_PortValue, pump_PortValue, L2_Position_PortValue, L2_LED_PortValue;

    Serial.println ("*********************************");
    Serial.println ("portOneValue  = "+binString(portOneValue));  
    Serial.println ("portTwoValue  = "+binString(portTwoValue));
    Serial.println ("L1_Position           = "+binString(L1_Position));  
    Serial.println ("L1_Position_PortValue = "+binString(chip0.readPort(0)));
    Serial.println ("L1_LED_State     = "+binString(L1_LED_State));    
    Serial.println ("L1_LED_PortValue = "+binString(chip0.readPort(1)));
    Serial.println ("pumpState      = "+binString(pumpState));   
    Serial.println ("pump_PortValue = "+binString(chip1.readPort(1)));
    Serial.println ("L2_Position           = "+binString(L2_Position));   
    Serial.println ("L2_Position_PortValue = "+binString(chip2.readPort(0)));
    Serial.println ("L2_LED_State     = "+binString(L2_LED_State));  
    Serial.println ("L2_LED_PortValue = "+binString(chip2.readPort(1)));
    Serial.println ("*********************************");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  Serial.println("Starting HD_Demo");
  chip0.begin();
  chip1.begin();
  chip2.begin();
  chip3.begin();
  configureChips();
  endSession();  // everything off
    
  init_10_mSec_Timer();
  portOneValue = chip1.readPort(0);          
  portTwoValue = chip3.readPort(0);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  showMenu();
}

void handleInputError(byte leverNum, byte portValue) {
  boolean recoveredFromError = false;   
  byte _portValue; 

  inputErrors++;
  // Print the error that got us here.
  Serial.print (String(millis()-startTime)+" I! Port ");
  Serial.print (String(leverNum)+" "+binString(portValue)); 

  for (int x = 0; x < 10; x++) {
    if (leverNum == 1) _portValue = chip1.readPort(0);
    else _portValue = chip3.readPort(0);
    if (_portValue == 0) Serial.print("I! ");         // Still has error
    else {
        recoveredFromError = true;
        inputRecoveries++;
        Serial.println(" ... recovered after "+String(x+1)+" attempt(s)");
        break;
    }
  }
  if (!recoveredFromError) {
      Serial.println();
      Serial.print("Trying to Reset Chips ... ");
      if (resetAndRecover()) Serial.println ("Reset");
      else Serial.println ("Failed on Output Error");
      if (chip1.readPort(0) != 0 && chip3.readPort(0) != 0) {
         Serial.println("Recovered from Input Error after resetAndRecover()");
         inputRecoveries++;
      }
      else {      
        Serial.println();
        Serial.println("Ending Session Because of Input Errors");
        endSession();
      }
  }   
}

void handleOutputError(){
  /* Report output errors; then check ten times to see if it can recover.  
     If it can't recover then endSession. Otherwise carry on...
  */
  boolean errorFound = false;
  boolean recoveredFromError = false;
  
  outputErrors++;
  Serial.println("OutputPorts Error ... Checking");
  reportOutputErrors();
      
  /* Reset Pumps - an abundance of caution. Nothing else will happen while in this loop
  pumpState = 0x00; 
  pumpStateL1 = 0x00;
  pumpStateL2 = 0x00;
  chip1.writePort(1,pumpState);
  // Reset L2_LED
  L2_LED_State = 0xFF;
  chip2.writePort(1,L2_LED_State);
  */
  for (int x = 0; x < 10; x++) {
    boolean errorFound = false;
    if (L1_Position  != chip0.readPort(0)) {
      errorFound = true;
      chip0.writePort(0,L1_Position);
    }
    if (L1_LED_State != chip0.readPort(1)) {
      errorFound = true;
      chip0.writePort(1,L1_LED_State);
    }
    if (pumpState    != chip1.readPort(1)) {
      errorFound = true;
      chip1.writePort(1,pumpState);
    }
    if (L2_Position  != chip2.readPort(0)) {
      errorFound = true;
      chip2.writePort(0,L2_Position);
    }
    if (L2_LED_State != chip2.readPort(1)) {
      errorFound = true;
      chip2.writePort(1,L2_LED_State);
    }
    if (errorFound) Serial.print("O! ");    // Returns true on error
    else {
        recoveredFromError = true;
        break;
      }
  }
  if (!recoveredFromError) {
      Serial.println();
      Serial.print("Attempting to reset output chips ... ");
      if (resetAndRecover()) Serial.println ("successful");
      else {
        Serial.println ("failed");
        Serial.println("Ending Session because of Output Errors");
        endSession();
      } 
  }
}

void reportOutputErrors() {
    byte L1_Position_PortValue, L1_LED_PortValue, pump_PortValue, L2_Position_PortValue, L2_LED_PortValue;
    boolean errorFound = false;
    
    L1_Position_PortValue = chip0.readPort(0);
    if (L1_Position != L1_Position_PortValue) {
        Serial.print("L1_Position != PortValue: "+binString(L1_Position));
        Serial.println(" vs "+binString(L1_Position_PortValue)); 
        }
    L1_LED_PortValue = chip0.readPort(1);
    if (L1_LED_State != L1_LED_PortValue) {
        Serial.print("L1_LED_State != PortValue: "+binString(L1_LED_State));
        Serial.println(" vs "+binString(L1_LED_PortValue));
        }
    pump_PortValue = chip1.readPort(1);
    if (pumpState != pump_PortValue) {
        Serial.print("pumpState != PortValue: "+binString(pumpState));   
        Serial.println(" vs "+binString(pump_PortValue));
        }
    L2_Position_PortValue = chip2.readPort(0);
    if (L2_Position != L2_Position_PortValue) {
        Serial.print("L2_Position != PortValue: = "+binString(L2_Position));   
        Serial.println(" vs "+binString(L2_Position_PortValue));        
        }
    L2_LED_PortValue = chip2.readPort(1);
    if (L2_LED_State != L2_LED_PortValue) {
        Serial.print("L2_LED_State     = "+binString(L2_LED_State));  
        Serial.println(" vs "+binString(L2_LED_PortValue));
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

void checkInputPort1() { 
   static byte oldPortOneValue = 255; 
   portOneValue = chip1.readPort(0);
   if (portTwoValue == 0) handleInputError(1,portOneValue);      // Input OR output Error
   else {
      if(portOneValue != oldPortOneValue) {
         L1_responses++; 
         Serial.print(millis());   
         Serial.print(" L1: ");
         Serial.println(portOneValue,BIN);
         oldPortOneValue = portOneValue; 
         // chip0.writePort(0,pinValues);
      }
   }
}

void checkInputPort2() { 
  /*  (1 << 7) - This shifts 1 to the left seven bits creating 
   *  a mask = 10000000. Together with the bitwise and (&) it evaluates each 
   *  bit in the byte.
   */
   long delta;
   micro1 = micros();
   portTwoValue = chip3.readPort(0);
   if (portTwoValue == 0) handleInputError(2,portTwoValue);      // Input OR output Error
   else {
      if (oldPortTwoValue != portTwoValue) {           // something changed
         L2_responses++; 
         for (int bits = 7; bits > -1; bits--) {
            if ((portTwoValue & (1 << bits)) != (oldPortTwoValue & (1 << bits))) {
               // something happened on this bit.
               if (sendTimeStamp) {
                   if (portTwoValue & (1 << bits)) {
                      TStamp tStamp = {bits, 'h', millis(), 0, 1};
                      printQueue.push(&tStamp);
                      //Serial.println("HIGH "+String(bits));
                   }
                   else {
                      TStamp tStamp = {bits, 'H', millis(), 1, 1};
                      printQueue.push(&tStamp);
                      // Serial.println("LOW "+String(bits));
                   } 
               }        
            }
         }
      }
      // At this point, nothing has been done to switch the pumps 
      oldPortTwoValue = portTwoValue;
      pumpStateL2 = (255-portTwoValue);
      pumpState = (pumpStateL1 | pumpStateL2);  // bitwise OR
      chip1.writePort(1,pumpState);
      L2_LED_State = portTwoValue;          // mirror pump state
      chip2.writePort(1,L2_LED_State);
 
      micro2 = micros();
      delta = micro2 - micro1;
      if (delta > maxDelta) maxDelta = delta;
      if (delta < minDelta) minDelta = delta;
   }
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

void handleInstruction()
{
   byte spaceIndex;
   String code1;
   String code2;
   int num;
   if (instruction.length() > 0)
   { // Serial.println(instruction);  
     spaceIndex = instruction.indexOf(' ');
     if (spaceIndex == -1)
     {  code1 = instruction;
        code2 = "";
     }
     else
     {  code1 = instruction.substring(0,spaceIndex);
        code2 = instruction.substring(spaceIndex+1,instruction.length());
     }
     num = code2.toInt();
     // Serial.println(code1+" "+num);
     if (code1 == "m") showMenu();     
     else if (code1 == "G") startSession();
     else if (code1 == "Q") endSession();
     else if (code1 == "P") bitSet(pumpStateL1,num);
     else if (code1 == "p") bitClear(pumpStateL1,num);
     else if (code1 == "T") sendTimeStamp = true;
     else if (code1 == "t") sendTimeStamp = false;
     else if (code1 == "H") handleInputError(2,portTwoValue);
     else if (code1 == "D") showDiagnosticData();
     else if (code1 == "R") showPorts();
     else if (code1 == "O") {
          if (checkOutputPorts()) handleOutputError();
          else Serial.println("Output Ports OK");
          }
     else if (code1 == "X") {
          if (resetAndRecover()) Serial.println ("Reset and Recovery successful");
          else Serial.println ("Reset and Recovery failed");
          }                             
   }
}

void getSerialInstruction()
{ while (Serial.available() > 0)        // repeat while something in the buffer
    { char aChar = Serial.read();
      if (aChar == '>')                 // end of instruction string
        {  handleInstruction();
           instruction = "";
        }
      else if (aChar == '<') instruction = "";         // beginning of instruction
      else instruction += aChar;
    }
}

void tick() {
   static long tickCounts = 0;
   tickCounts++;
   if (tickCounts >= 100) {     // each second   
      tickCounts = 0;
      if (sessionRunning) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        if (checkOutputPorts()) handleOutputError();;
      }
   }
   if (sessionRunning) {
      checkInputPort1();
      checkInputPort2();
   }
   getSerialInstruction(); 
   sendOneTimeStamp();
}

void loop() {
   if (tickFlag)
   {  tickFlag = false;
      tick();
   }
}
