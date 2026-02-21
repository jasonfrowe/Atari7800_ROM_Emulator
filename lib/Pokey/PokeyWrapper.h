#ifndef POKEY_WRAPPER_H
#define POKEY_WRAPPER_H

#include "pokey.h" 
#include <Arduino.h>

class PokeyWrapper {
private:
    Pokey m_pokey;

public:
    PokeyWrapper() {
        // Leave this EMPTY. No hardware or complex logic here.
    }

    void begin() {
        m_pokey.Reset(); // Safe to call now
    }

    void tickStep() {
        m_pokey.TickStep();
    }

    uint8_t getOutput() {
        return m_pokey.GetOutput(); 
    }

    void writeRegister(uint8_t addr, uint8_t val) {
        m_pokey.Write(addr & 0x0F, val);
    }
};

#endif