/*
 * MCP23S17.h - Library for controllong MCP23S17 Port Expanders
 * 
 * Forked from Majenko MCP23S17 library: https://github.com/MajenkoLibraries/MCP23S17
 * 
 */

#ifndef MCP23S17_h
#define MCP23S17_h

#include "Arduino.h"

class MCP23S17 {
    private:
        uint8_t _cs;    /*! Chip select pin */
        uint8_t _addr;  /*! 3-bit chip address */
    
        uint8_t _reg[22];   /*! Local mirrors of the 22 internal registers of the MCP23S17 chip */

        enum {
            IODIRA,     IODIRB,
            IPOLA,      IPOLB,
            GPINTENA,   GPINTENB,
            DEFVALA,    DEFVALB,
            INTCONA,    INTCONB,
            IOCONA,     IOCONB,
            GPPUA,      GPPUB,
            INTFA,      INTFB,
            INTCAPA,    INTCAPB,
            GPIOA,      GPIOB,
            OLATA,      OLATB
        };

        void readRegister(uint8_t addr); 
        void writeRegister(uint8_t addr);
        void readAll();
        void writeAll();
    
    public:
        MCP23S17(uint8_t cs, uint8_t addr);
        void begin();
        void pinMode(uint8_t pin, uint8_t mode);
        void digitalWrite(uint8_t pin, uint8_t value);
        uint8_t digitalRead(uint8_t pin);

        uint8_t readPort(uint8_t port);
        uint16_t readPort();
        void writePort(uint8_t port, uint8_t val);
        void writePort(uint16_t val);
        void enableInterrupt(uint8_t pin, uint8_t type);
        void disableInterrupt(uint8_t pin);
        void setMirror(boolean m);
        uint16_t getInterruptPins();
        uint16_t getInterruptValue();
        void setInterruptLevel(uint8_t level);
        void setInterruptOD(boolean openDrain);
};

#endif
