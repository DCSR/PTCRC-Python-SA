
/*
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
#include <SPI.h>        // Arduino Library SPI.h
#include "MCP23S17.h"   // Majenko MCP23S17 library

//**************************************

const uint8_t chipSelect = 10;  
MCP23S17 chip0(chipSelect, 0); // L1 retract map to pins 0..7; L1 LEDs map to pins 8..15 
MCP23S17 chip1(chipSelect, 1); // L1 inputs map to pins 0..7; Pumps map to pins 8..15
MCP23S17 chip2(chipSelect, 2); // L2 retract map to pins 0..7; LEDs map to pins 8..15
MCP23S17 chip3(chipSelect, 3); // L2 inputs map to pins 0..7; AUX output map to pins 8..15
byte portOneValue, portTwoValue, oldPortTwoValue = 255;
byte cycleMode = 1;
String instruction;
const uint8_t ledPin = 5;

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
  Serial.println ("MCP23S17_Demo_M0.ino.");
  Serial.println ("<m> - Show this Menu");
  Serial.println ("<X> - turns on Pump 1");
  Serial.println ("<x> - turns off Pump 1"); 
  Serial.println ("<c 1> - cycle mode 1: each pin");
  Serial.println ("<c 2> - cycle mode 2: each port");
  Serial.println ("<c 3> - cycle mode 3: each chip");
  Serial.println ("<P>   - All Pumps On");
  Serial.println ("<p> - All Pumps Off");
}

void everythingOff() {
     chip0.writePort(0,0xFF);   // Retract L1
     chip0.writePort(1,0xFF);   // L1 LED Off
     chip1.writePort(1,0x00);   // Pumps Off
     chip2.writePort(0,0xFF);   // Extend L2
     chip2.writePort(1,0xFF);   // L2 LED On
     chip3.writePort(1,0xFF);   // Aux Off
}

void pumpsOff(){
  cycleMode = 0;  
  Serial.println("Turning Pumps Off");
  chip1.writePort(1,0x00);     // Pumps : Off = LOW
}

void pumpsOn(){
  cycleMode = 0;  
  Serial.println("Turning Pumps On");
  chip1.writePort(1,0xFF);     // Pumps : On = HIGH
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.print("Starting HD_Demo");
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

  Serial.println(portOneValue,BIN); 
  pinMode(ledPin, OUTPUT);                // GPIO 10
  showMenu();
}


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

*/

void checkInputPort1() { 
   static byte oldPortOneValue = 255; 
   portOneValue = chip1.readPort(0);
   if(portOneValue != oldPortOneValue) {   
      Serial.print("L1:");
      Serial.println(portOneValue,BIN);
      oldPortOneValue = portOneValue; 
      // chip0.writePort(0,pinValues);
   }
}

void checkInputPort2() {  
   portTwoValue = chip3.readPort(0);
   if(portTwoValue != oldPortTwoValue) {
      Serial.print(portTwoValue);
      Serial.print(" ");
      Serial.println(oldPortTwoValue); 
      for (byte i = 0; i < 8; i++) {
        if (bitRead(portTwoValue,i) != (bitRead(oldPortTwoValue,i))) Serial.print(i);
        else Serial.print("?");
      oldPortTwoValue = portTwoValue;
      }
  }
  chip1.writePort(1,255-portTwoValue);  
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
     if (code1 == "X") chip1.digitalWrite(8,0); 
     else if (code1 == "x") chip1.digitalWrite(8,1);
     
     else if (code1 == "1") chip0.writePort(0,0x00);
     else if (code1 == "2") chip0.writePort(0,0xFF);     
     else if (code1 == "3") chip0.writePort(1,0x00);    // L1 LED On
     else if (code1 == "4") chip0.writePort(1,0xFF);    // L1 LED Off

     // else if code1 == "5") chip1.writePort(0,0x00);
     // else if (code1 == "6") chip1.writePort(0,0xFF);     
     else if (code1 == "5") chip1.writePort(1,0);    // Pumps Off
     else if (code1 == "6") chip1.writePort(1,3);    // Pumps  1 and 2 On

     else if (code1 == "7") chip2.writePort(0,0x00);    // L2 Extend
     else if (code1 == "8") chip2.writePort(0,0xFF);    // L2 Retract
     else if (code1 == "9") chip2.writePort(1,0x00);    // L2 LED On
     else if (code1 == "A") chip2.writePort(1,0xFF);    // L2 LED Off


     
     // else if (code1 == "c") cycleMode = num;
     // else if (code1 == "I") timeCheckInputPort();
     // else if (code1 == "8") timeOutput(1);
     // else if (code1 == "9") timeOutput(2);
     // else if (code1 == "m") showMenu();
     else if (code1 == "P") pumpsOn();
     else if (code1 == "p") pumpsOff();
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

void tick() {
   static long tickCounts = 0;
   tickCounts++;
   if (tickCounts >= 100) {     // every seconds
      tickCounts = 0;
      cycleOutputs(); 
      digitalWrite(ledPin, !digitalRead(ledPin));
      // Serial.println(millis());
      // Serial.print(".");
   }
   checkInputPort1();
   checkInputPort2();
   getSerialInstruction(); 
}

void loop() {
   if (tickFlag)
   {  tickFlag = false;
      tick();
   }
}
