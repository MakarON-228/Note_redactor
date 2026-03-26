#pragma once

#include "MusicSymbol.h"

class TimeSignature : public MusicSymbol {
public:
    TimeSignature(int xSlot, int staffIndex, int numerator, int denominator)
        : MusicSymbol(SymbolType::TimeSignature),
          m_xSlot(xSlot),
          m_staffIndex(staffIndex),
          m_numerator(numerator),
          m_denominator(denominator) {}

    int xSlot() const { return m_xSlot; }
    int staffIndex() const { return m_staffIndex; }
    int numerator() const { return m_numerator; }
    int denominator() const { return m_denominator; }

    void setXSlot(int slot) { m_xSlot = (slot < 0) ? 0 : slot; }
    void setStaffIndex(int staffIndex) { m_staffIndex = (staffIndex < 0) ? 0 : staffIndex; }

    void setNumerator(int n) { m_numerator = n; }
    void setDenominator(int d) { m_denominator = d; }

private:
    int m_xSlot;
    int m_staffIndex;
    int m_numerator;
    int m_denominator;
};

