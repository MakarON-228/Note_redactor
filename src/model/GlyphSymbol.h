#pragma once

#include "MusicSymbol.h"
#include "Note.h"

class GlyphSymbol : public MusicSymbol {
public:
    GlyphSymbol(SymbolType symbolType, int xSlot, int staffIndex, int staffStep, Note::Duration restDuration = Note::Duration::Whole)
        : MusicSymbol(symbolType),
          m_xSlot(xSlot),
          m_staffIndex(staffIndex),
          m_staffStep(staffStep),
          m_restDuration(restDuration) {}

    int xSlot() const { return m_xSlot; }
    int staffIndex() const { return m_staffIndex; }
    int staffStep() const { return m_staffStep; }

    Note::Duration restDuration() const { return m_restDuration; }
    void setRestDuration(Note::Duration duration) { m_restDuration = duration; }

    void setXSlot(int slot) { m_xSlot = (slot < 0) ? 0 : slot; }
    void setStaffIndex(int staffIndex) { m_staffIndex = (staffIndex < 0) ? 0 : staffIndex; }
    void setStaffStep(int step) { m_staffStep = step; }

private:
    int m_xSlot;
    int m_staffIndex;
    int m_staffStep;
    Note::Duration m_restDuration;
};
