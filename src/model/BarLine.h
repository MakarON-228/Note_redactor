#pragma once

#include "MusicSymbol.h"

class BarLine : public MusicSymbol {
public:
    explicit BarLine(int xSlot, int staffIndex)
        : MusicSymbol(SymbolType::BarLine),
          m_xSlot(xSlot),
          m_staffIndex(staffIndex) {}

    // Convenience overload: defaults to first staff row.
    explicit BarLine(int xSlot)
        : BarLine(xSlot, 0) {}

    int xSlot() const { return m_xSlot; }
    int staffIndex() const { return m_staffIndex; }
    void setXSlot(int slot) { m_xSlot = (slot < 0) ? 0 : slot; }
    void setStaffIndex(int staffIndex) { m_staffIndex = (staffIndex < 0) ? 0 : staffIndex; }
    void moveHorizontal(int deltaSlots) { setXSlot(m_xSlot + deltaSlots); }

private:
    int m_xSlot;
    int m_staffIndex;
};
