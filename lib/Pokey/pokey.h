#ifndef POKEY_H
#define POKEY_H

#include "typedefs.h"

class Pokey {
public:
    Pokey();
    void Reset();
    void Write(uint8 addr, uint8 val);
    bool TickStep();
    uint8 GetOutput() { return m_cachedOutput; }

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

    uint8_t m_cachedOutput;
    uint8_t m_tickStep;
    uint32_t m_tempTotal;

    void UpdatePoly();
};

#endif // POKEY_H
