#ifndef POKEY_H
#define POKEY_H

#include "typedefs.h"

class Pokey {
public:
    Pokey();
    void Reset();
    void Write(uint8 addr, uint8 val);
    bool TickStep();
    uint8 GetOutput() const { return m_cachedOutput; }

private:
    uint8 m_regs[16];
    
    // Timer state
    uint8  m_counter[4];
    uint8  m_divisor[4];
    uint8  m_output[4];
    
    // Polynomial state (LFSRs)
    uint32 m_poly4;
    uint32 m_poly5;
    uint32 m_poly9;
    uint32 m_poly17;
    uint8  m_polyState; // Combined current bits (1=poly4, 2=5, 4=9, 8=17)

    uint8_t m_cachedOutput;
    uint8_t m_tickStep;
    uint32_t m_tempTotal;
    uint8_t  m_clock64;      // Divider for 64kHz mode (28 ticks)
    uint32_t m_postCounter;  // Counter for 3-second POST chirp

    void UpdatePoly();
    static const uint8_t DistortionLUT[8][16];
};

#endif // POKEY_H
