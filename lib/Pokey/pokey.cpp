#include "pokey.h"
#include <string.h>

const uint8_t Pokey::DistortionLUT[8][16] = {
    {0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1}, // 0: 5 & 17 (Bits 1 & 3)
    {0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1}, // 1: 5 (Bit 1)
    {0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,1}, // 2: 4 & 17 (Bits 0 & 3)
    {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1}, // 3: 4 (Bit 0)
    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1}, // 4: 17 (Bit 3)
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // 5: Pure
    {0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1}, // 6: 9 (Bit 2)
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}  // 7: Pure
};

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
    m_polyState = 0;
    m_postCounter = 0;
    
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
    } else if (addr == 9) { // STIMER
        for (int i = 0; i < 4; i++) {
            m_counter[i] = m_divisor[i];
        }
    }
}

void Pokey::UpdatePoly() {
    uint32 bit4 = ((m_poly4 >> 3) ^ (m_poly4 >> 2)) & 1;
    m_poly4 = ((m_poly4 << 1) | bit4) & 0x0F;

    uint32 bit5 = ((m_poly5 >> 4) ^ (m_poly5 >> 2)) & 1;
    m_poly5 = ((m_poly5 << 1) | bit5) & 0x1F;

    uint32 bit9 = ((m_poly9 >> 8) ^ (m_poly9 >> 4)) & 1;
    m_poly9 = ((m_poly9 << 1) | bit9) & 0x1FF;

    uint32 bit17 = ((m_poly17 >> 16) ^ (m_poly17 >> 13)) & 1;
    m_poly17 = ((m_poly17 << 1) | bit17) & 0x1FFFF;

    m_polyState = (m_poly4 & 1) | ((m_poly5 & 1) << 1) | ((m_poly9 & 1) << 2) | ((m_poly17 & 1) << 3);
}

bool Pokey::TickStep() {
    switch (m_tickStep) {
        case 0:
            UpdatePoly();
            m_tempTotal = 0;

            m_tickStep = 1;
            break;

        case 1: case 2: case 3: case 4: {
            int i = m_tickStep - 1;
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

        case 5: case 6: case 7: case 8: {
            int i = m_tickStep - 5;
            uint8 audc = m_regs[i * 2 + 1];
            if (m_output[i]) {
                uint8 dist = (audc >> 5) & 0x07;
                if (DistortionLUT[dist][m_polyState]) {
                    m_tempTotal += (audc & 0x0F);
                }
            }

            if (m_tickStep == 8) {
                // Done. Scale 0-60 to 0-255.
                m_cachedOutput = (uint8)((m_tempTotal << 2) + (m_tempTotal >> 2));
                m_tickStep = 0;
                return true; 
            }
            m_tickStep++;
            break;
        }
    }
    return false;
}
