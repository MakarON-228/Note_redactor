#pragma once

#include "MusicSymbol.h"

class Dot : public MusicSymbol {
public:
    explicit Dot(int xSlot, int staffIndex, int staffStep)
        : MusicSymbol(SymbolType::Dot),
          m_xSlot(xSlot),
          m_staffStep(staffStep),
          m_staffIndex(staffIndex) {}

    // Convenience overload: defaults to first staff row.
    explicit Dot(int xSlot, int staffStep)
        : Dot(xSlot, 0, staffStep) {}

    int xSlot() const { return m_xSlot; }
    int staffIndex() const { return m_staffIndex; }
    int staffStep() const { return m_staffStep; }
    void setXSlot(int slot) { m_xSlot = (slot < 0) ? 0 : slot; }
    void setStaffIndex(int staffIndex) { m_staffIndex = (staffIndex < 0) ? 0 : staffIndex; }
    void setStaffStep(int step) { m_staffStep = step; }
    void moveHorizontal(int deltaSlots) { setXSlot(m_xSlot + deltaSlots); }

private:
    int m_xSlot;
    int m_staffIndex;
    int m_staffStep;
};