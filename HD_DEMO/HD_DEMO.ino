
/*
 * See: 
 * http://www.handsontec.com/dataspecs/module/8Ch-relay.pdf
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
boolean recoveringFromError = false;
boolean sendTimeStamp = false;
byte diffCriteria = 1;   // used for bitread() error checking
  

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



void showMenu () {
  Serial.println ("HD_Demo.ino.");
  Serial.println ("<m> - Show this Menu");
  Serial.println ("<G> - Start HD Session - Feather LED should blink");
  Serial.println ("<Q> - End HD Session"); 
  Serial.println ("<P num> - Switch Pump On");
  Serial.println ("<p num> - Switch Pump Off");
  Serial.println ("<E> - everythingOff()"); 
  Serial.println ("<T> - send timeStamps");
  Serial.println ("<t> - suppress timeStamps");   
  Serial.println ("<H> - trigger handleError()");
  Serial.println ("<D> - showDiagnosticData()");
  Serial.println ("<R> - showOutputPorts()");
  Serial.println ("<O> - outputErrorCheck()");
}

void startSession() {
  L1_Position = 0x00;              // Extend L1
  chip0.writePort(0,L1_Position);  
  L1_LED_State = 0xFF;             // L1 LED Off
  chip0.writePort(1,L1_LED_State);         
  L2_Position = 0x00;              // Extend L2
  chip2.writePort(0,L2_Position);
  L2_LED_State = 0xFF;             // L2 LED Off
  chip2.writePort(1,L2_LED_State); 
  digitalWrite(LED_BUILTIN, HIGH);    
  sessionRunning = true;
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
  digitalWrite(LED_BUILTIN, LOW);   
  sessionRunning = false;
}

void everythingOff() {
    L1_Position = 0xFF;                // Retract L1
    chip0.writePort(0,L1_Position);
    L1_LED_State = 0xFF;               // L1 LED Off
    chip0.writePort(1,L1_LED_State);
    pumpState = 0x00; 
    pumpStateL1 = 0x00;
    pumpStateL2 = 0x00;
    chip1.writePort(1,pumpState);      // Pumps Off
    L2_Position = 0xFF;                // Retract L2 
    chip2.writePort(0,L2_Position);   
    L2_LED_State = 0xFF;     
    chip2.writePort(1,L2_LED_State);   // L2 LED Off
    chip3.writePort(1,0xFF);           // Aux Off
    sessionRunning = false;
}

void handleError(int code) {    
    Serial.println ("******  !!!! ******");
    Serial.println ("Error detected at "+String(millis())+" mSec");
    if (code == 1) Serial.println ("chip3.readPort(0) = 0x00");
    else Serial.println (String(code)+" different bits changed");
    // In FeatherM0, add a timestamp for error detection. "!"
    showOutputPorts();
    // Reset Pumps
    pumpState = pumpStateL1;    // Ignore pumpStateL2
    chip1.writePort(1,pumpState);
    // Reset L2_LED
    L2_LED_State = 0xFF;
    chip2.writePort(1,L2_LED_State);
    recoveringFromError = true;
}

void recoverFromError() { 
  portTwoValue = chip3.readPort(0);
  if (portTwoValue == 255) {
    oldPortTwoValue = portTwoValue;
    Serial.println ("Recovered at "+String(millis()));
    outputErrorCheck();
    recoveringFromError = false;  // start runnning again
  }
}

void showBits(int c) {
  for (int bits = 7; bits > -1; bits--) {
    // Compare bits 7-0 in byte
    if (c & (1 << bits)) {
      Serial.print("1");
    }
    else {
      Serial.print("0");
    }
  }
  Serial.println(" ");
}

void showDiagnosticData() {
  Serial.println ("***** Diagnostic Data *****");
  Serial.print ("Min and Max Deltas in microSec: "); 
  Serial.println (String(minDelta)+" "+String(maxDelta));
  minDelta = 1000;
  maxDelta = 0;
}

void outputErrorCheck() {
    boolean errorFound = false;
    if (L1_Position != chip0.readPort(0)) errorFound = true;
    if (L1_LED_State != chip0.readPort(1)) errorFound = true;
    if (pumpState != chip1.readPort(1)) errorFound = true;
    if (L2_Position != chip2.readPort(0)) errorFound = true;
    if (L2_LED_State != chip2.readPort(1)) errorFound = true;
    if (errorFound) showOutputPorts();
    else Serial.println("Output OK");
}

void showOutputPorts() {
  byte L1_Position_PortValue, L1_LED_PortValue, pump_PortValue, L2_Position_PortValue, L2_LED_PortValue;

    Serial.println ("*******************************");
    Serial.print ("portOneValue  = ");
    showBits(portOneValue);    
    Serial.print ("portTwoValue  = ");
    showBits(portTwoValue);

    // Compare L1_Position and L1_Position_PortValue
    Serial.print("L1_Position           = ");
    showBits(L1_Position);     
    L1_Position_PortValue = chip0.readPort(0);
    Serial.print("L1_Position_PortValue = ");
    showBits(L1_Position_PortValue);
  
    // Compare L1_LED_State and L1_LED_PortValue
    Serial.print("L1_LED_State     = ");
    showBits(L1_LED_State);     
    L1_LED_PortValue = chip0.readPort(1);
    Serial.print("L1_LED_PortValue = ");
    showBits(L1_LED_PortValue);
 
    // Compare pumpState and pump_PortValue
    Serial.print("pumpState      = ");
    showBits(pumpState);    
    pump_PortValue = chip1.readPort(1);
    Serial.print("pump_PortValue = ");
    showBits(pump_PortValue);
  
    // Compare L2_Position and L2_Position_PortValue
    Serial.print("L2_Position           = ");
    showBits(L2_Position);     
    L2_Position_PortValue = chip2.readPort(0);
    Serial.print("L2_Position_PortValue = ");
    showBits(L2_Position_PortValue);
  
    // Compare L2_LED_State and L2_LED_PortValue
    Serial.print("L2_LED_State     = ");
    showBits(L2_LED_State);     
    L2_LED_PortValue = chip2.readPort(1);
    Serial.print("L2_LED_PortValue = ");
    showBits(L2_LED_PortValue);

    Serial.println ("*******************************");
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
  everythingOff();
    
  init_10_mSec_Timer();
  portOneValue = chip1.readPort(0);          
  portTwoValue = chip3.readPort(0);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  showMenu();
}

void checkInputPort1() { 
   static byte oldPortOneValue = 255; 
   portOneValue = chip1.readPort(0);
   if(portOneValue != oldPortOneValue) {
      Serial.print(millis());   
      Serial.print(" L1: ");
      Serial.println(portOneValue,BIN);
      oldPortOneValue = portOneValue; 
      // chip0.writePort(0,pinValues);
   }
}

void checkInputPort2() { 
  /*  (1 << 7) - This shifts 1 to the left seven bits creating 
   *  a mask = 10000000. Together with the bitwise and (&) it evaluates each 
   *  bit in the byte.
   */
   micro1 = micros();
   byte diff = 0;
   long delta;
   if (recoveringFromError) recoverFromError();
   else {
       portTwoValue = chip3.readPort(0);
       if (portTwoValue == 0) handleError(1);
       else {
          if (oldPortTwoValue != portTwoValue) {           // something changed
             for (int bits = 7; bits > -1; bits--) {
                if ((portTwoValue & (1 << bits)) != (oldPortTwoValue & (1 << bits))) {
                   // something happened on this bit.
                   diff = diff+1;                            // count the number of bit differences
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
          if (diff > 1) handleError(diff);      // abort
          else {                                // no errors
               oldPortTwoValue = portTwoValue;
               pumpStateL2 = (255-portTwoValue);
               pumpState = (pumpStateL1 | pumpStateL2);  // bitwise OR
               if (sessionRunning) {
                   chip1.writePort(1,pumpState);
                   L2_LED_State = portTwoValue;          // mirror pump state
                   chip2.writePort(1,L2_LED_State);
               } 
          }
       micro2 = micros();
       delta = micro2 - micro1;
       if (delta > maxDelta) maxDelta = delta;
       if (delta < minDelta) minDelta = delta;
       }
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
     else if (code1 == "E") everythingOff();
     else if (code1 == "T") sendTimeStamp = true;
     else if (code1 == "t") sendTimeStamp = false;
     else if (code1 == "H") handleError(5);
     else if (code1 == "D") showDiagnosticData();
     else if (code1 == "R") showOutputPorts();
     else if (code1 == "O") outputErrorCheck();
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
   if (tickCounts >= 100) {     // every seconds   
      tickCounts = 0;
      if (sessionRunning) digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      // Serial.println(millis());
      // Serial.print(".");
   }
   checkInputPort1();
   checkInputPort2();
   getSerialInstruction(); 
   sendOneTimeStamp();
}

void loop() {
   if (tickFlag)
   {  tickFlag = false;
      tick();
   }
}

/*
void timeOutput(int mode){
  long delta, micro1, N;
  N = 0;
  if (mode == 1) {
    Serial.println("Timing 25600 Pump swtchings using digitalWrite");
    micro1 = micros();
    for (long i = 0; i <= 12800; i++){  
        chip1.digitalWrite(8,1);
        N++;
        chip1.digitalWrite(8,0);
        N++;
    }
    delta = micros() - micro1;
  }
  else if (mode == 2) {
    Serial.println("Timing 25600 Pump swtchings using writePort");
    micro1 = micros();
    for (long T = 0; T <= 99; T++) {
      // Serial.print(T);
      // Serial.println(":");
      for (long i = 0; i <= 255; i++){
          chip1.writePort(1,i);
          N++;
          // Serial.println(N);
      }
    }
    delta = micros() - micro1;
  }   
  Serial.print("Average time = ");
    float t = (float)delta / (float)N;
  Serial.print(t, 2);
  Serial.print(" uSec over "); 
  Serial.print(N);
  Serial.println(" switchings");
}

/*
void timeCheckInputPort(){
  long delta, micro1;
  micro1 = micros();
  pinValues = chip3.readPort(0);
  delta = micros() - micro1;
  Serial.print("CheckInputPort (uSec) ");
  Serial.println(delta);
}

void cycleOutputs () {
  static byte pin = 15;
  static byte blockNum = 0;
  static boolean state = true;
  byte val;
  int block[4] = {0xFFF0,0xFF0F,0xF0FF,0x0FFF};
  if (cycleMode == 1) {
    chip0.digitalWrite(pin,HIGH);   // turn something OFF
    chip0.digitalWrite(pin+8,HIGH);   // turn something OFF
    chip1.digitalWrite(pin,HIGH);   // turn something OFF   
    //chip2.digitalWrite(pin,HIGH);   // turn something OFF 
    pin++;
    if (pin > 8) pin = 0; 
    chip0.digitalWrite(pin,LOW);   // turn something ON
    chip0.digitalWrite(pin+8,LOW);   // turn something ON
    chip1.digitalWrite(pin,LOW);   // turn something ON
    //chip2.digitalWrite(pin,LOW);   // turn something OFF 
  }
  else if (cycleMode == 2) {
    state = !state;               // change state
    if (state) val = 0xF0;
    else val = 0x0F;
    chip0.writePort(0,val);
    chip0.writePort(1,val);
    chip1.writePort(0,val);
    // chip1.writePort(1,val);
    // chip2.writePort(0,val);
    //chip2.writePort(1,val);
  }
  else if (cycleMode == 3) {
    chip0.writePort(block[blockNum]);
    // chip1.writePort(block[blockNum]);
    // chip2.writePort(block[blockNum]);
    blockNum++;
    if (blockNum == 4) blockNum = 0;
  }
}

*/
