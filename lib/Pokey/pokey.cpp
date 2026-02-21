#include "pokey.h"
#include <string.h>

Pokey::Pokey() {
    Reset();
}

void Pokey::Reset() {
    memset(m_regs, 0, sizeof(m_regs));
    memset(m_counter, 0, sizeof(m_counter));
    memset(m_divisor, 0, sizeof(m_divisor));
    memset(m_output, 0, sizeof(m_output));
    m_cachedOutput = 0;
    m_tickStep = 0;
    m_tempTotal = 0;
    
    m_poly4 = 0x0F;
    m_poly5 = 0x1F;
    m_poly9 = 0x1FF;
    m_poly17 = 0x1FFFF;
}

void Pokey::Write(uint8 addr, uint8 val) {
    addr &= 0x0F;
    m_regs[addr] = val;
    
    if (addr < 8 && (addr & 1) == 0) {
        m_divisor[addr >> 1] = val;
    }
}

void Pokey::UpdatePoly() {
    // Fast LFSR Updates
    uint32 bit4 = ((m_poly4 >> 3) ^ (m_poly4 >> 2)) & 1;
    m_poly4 = ((m_poly4 << 1) | bit4) & 0x0F;

    uint32 bit5 = ((m_poly5 >> 4) ^ (m_poly5 >> 2)) & 1;
    m_poly5 = ((m_poly5 << 1) | bit5) & 0x1F;

    uint32 bit9 = ((m_poly9 >> 8) ^ (m_poly9 >> 4)) & 1;
    m_poly9 = ((m_poly9 << 1) | bit9) & 0x1FF;

    uint32 bit17 = ((m_poly17 >> 16) ^ (m_poly17 >> 13)) & 1;
    m_poly17 = ((m_poly17 << 1) | bit17) & 0x1FFFF;
}

bool Pokey::TickStep() {
    switch (m_tickStep) {
        case 0:
            UpdatePoly();
            m_tempTotal = 0;
            m_tickStep = 1;
            break;

        case 1: case 3: case 5: case 7: {
            int i = (m_tickStep - 1) >> 1;
            uint8 audc = m_regs[i * 2 + 1];
            if (!(audc & 0x10)) {
                if (m_counter[i] == 0) {
                    m_counter[i] = m_divisor[i];
                    m_output[i] ^= 1;
                } else {
                    m_counter[i]--;
                }
            } else {
                m_output[i] = 1;
            }
            m_tickStep++;
            break;
        }

        case 2: case 4: case 6: case 8: {
            int i = (m_tickStep - 2) >> 1;
            uint8 audc = m_regs[i * 2 + 1];
            bool signal = m_output[i];
            
            if (signal) {
                uint8 dist = (audc >> 5) & 0x07;
                switch (dist) {
                    case 0: signal &= (m_poly5  & 1) && (m_poly17 & 1); break;
                    case 1: signal &= (m_poly5  & 1); break;
                    case 2: signal &= (m_poly4  & 1) && (m_poly17 & 1); break;
                    case 3: signal &= (m_poly4  & 1); break;
                    case 4: signal &= (m_poly17 & 1); break;
                    case 5: break; 
                    case 6: signal &= (m_poly9  & 1); break;
                    case 7: break; 
                }
                if (signal) m_tempTotal += (audc & 0x0F);
            }

            if (m_tickStep == 8) {
                // Final Step: Scale and reset
                m_cachedOutput = (uint8)((m_tempTotal << 2) + (m_tempTotal >> 2));
                m_tickStep = 0;
                return true; // Cycle complete
            } else {
                m_tickStep++;
            }
            break;
        }
    }
    return false;
}
