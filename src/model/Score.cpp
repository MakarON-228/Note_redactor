#include "Score.h"

#include <cmath>

std::shared_ptr<Note> Score::addNote(int xSlot, int staffIndex, int staffStep) {
    auto note = std::make_shared<Note>(xSlot, staffIndex, staffStep);
    m_symbols.push_back(note);
    return note;
}

std::shared_ptr<Note> Score::addNote(int xSlot, int staffStep) {
    return addNote(xSlot, 0, staffStep);
}

std::shared_ptr<Note> Score::addNote(int xSlot, int staffIndex, int staffStep, Note::Duration duration) {
    auto note = std::make_shared<Note>(xSlot, staffIndex, staffStep, duration);
    m_symbols.push_back(note);
    return note;
}

std::shared_ptr<Note> Score::addNote(int xSlot, int staffStep, Note::Duration duration) {
    return addNote(xSlot, 0, staffStep, duration);
}

std::shared_ptr<BarLine> Score::addBarLine(int xSlot, int staffIndex) {
    auto barLine = std::make_shared<BarLine>(xSlot, staffIndex);
    m_symbols.push_back(barLine);
    return barLine;
}

std::shared_ptr<BarLine> Score::addBarLine(int xSlot) {
    return addBarLine(xSlot, 0);
}

std::shared_ptr<TimeSignature> Score::addTimeSignature(int xSlot, int staffIndex, int numerator, int denominator) {
    auto ts = std::make_shared<TimeSignature>(xSlot, staffIndex, numerator, denominator);
    m_symbols.push_back(ts);
    return ts;
}

std::shared_ptr<TimeSignature> Score::addTimeSignature(int xSlot, int numerator, int denominator) {
    return addTimeSignature(xSlot, 0, numerator, denominator);
}

std::shared_ptr<GlyphSymbol> Score::addGlyph(MusicSymbol::SymbolType type, int xSlot, int staffIndex, int staffStep) {
    auto glyph = std::make_shared<GlyphSymbol>(type, xSlot, staffIndex, staffStep);
    m_symbols.push_back(glyph);
    return glyph;
}

std::shared_ptr<GlyphSymbol> Score::addGlyph(MusicSymbol::SymbolType type, int xSlot, int staffStep) {
    return addGlyph(type, xSlot, 0, staffStep);
}

std::shared_ptr<GlyphSymbol> Score::addGlyph(MusicSymbol::SymbolType type, int xSlot, int staffIndex, int staffStep, Note::Duration restDuration) {
    auto glyph = std::make_shared<GlyphSymbol>(type, xSlot, staffIndex, staffStep, restDuration);
    m_symbols.push_back(glyph);
    return glyph;
}

std::shared_ptr<GlyphSymbol> Score::addGlyph(MusicSymbol::SymbolType type, int xSlot, int staffStep, Note::Duration restDuration) {
    return addGlyph(type, xSlot, 0, staffStep, restDuration);
}

bool Score::removeNoteAt(int xSlot, int staffIndex, int staffStep, int slotTolerance, int stepTolerance) {
    for (int i = 0; i < m_symbols.size(); ++i) {
        auto note = std::dynamic_pointer_cast<Note>(m_symbols[i]);
        if (!note) {
            continue;
        }

        const bool inSlotRange = std::abs(note->xSlot() - xSlot) <= slotTolerance;
        const bool inStaffIndex = note->staffIndex() == staffIndex;
        const bool inStepRange = std::abs(note->staffStep() - staffStep) <= stepTolerance;
        if (inSlotRange && inStaffIndex && inStepRange) {
            m_symbols.remove(i);
            return true;
        }
    }

    return false;
}

bool Score::removeNoteAt(int xSlot, int staffStep, int slotTolerance, int stepTolerance) {
    return removeNoteAt(xSlot, 0, staffStep, slotTolerance, stepTolerance);
}

bool Score::removeBarLineAt(int xSlot, int staffIndex, int slotTolerance) {
    for (int i = 0; i < m_symbols.size(); ++i) {
        auto barLine = std::dynamic_pointer_cast<BarLine>(m_symbols[i]);
        if (!barLine) {
            continue;
        }

        const bool inSlotRange = std::abs(barLine->xSlot() - xSlot) <= slotTolerance;
        const bool inStaffIndex = barLine->staffIndex() == staffIndex;
        if (inSlotRange && inStaffIndex) {
            m_symbols.remove(i);
            return true;
        }
    }

    return false;
}

bool Score::removeBarLineAt(int xSlot, int slotTolerance) {
    return removeBarLineAt(xSlot, 0, slotTolerance);
}

bool Score::removeTimeSignatureAt(int xSlot, int staffIndex, int slotTolerance) {
    for (int i = 0; i < m_symbols.size(); ++i) {
        auto ts = std::dynamic_pointer_cast<TimeSignature>(m_symbols[i]);
        if (!ts) {
            continue;
        }

        const bool inSlotRange = std::abs(ts->xSlot() - xSlot) <= slotTolerance;
        const bool inStaffIndex = ts->staffIndex() == staffIndex;
        if (inSlotRange && inStaffIndex) {
            m_symbols.remove(i);
            return true;
        }
    }

    return false;
}

bool Score::removeTimeSignatureAt(int xSlot, int slotTolerance) {
    return removeTimeSignatureAt(xSlot, 0, slotTolerance);
}

bool Score::removeSymbol(const std::shared_ptr<MusicSymbol>& symbol) {
    if (!symbol) {
        return false;
    }

    for (int i = 0; i < m_symbols.size(); ++i) {
        if (m_symbols[i] == symbol) {
            m_symbols.remove(i);
            return true;
        }
    }
    return false;
}

std::shared_ptr<Note> Score::noteAt(int xSlot, int staffIndex, int staffStep, int slotTolerance, int stepTolerance) {
    for (int i = m_symbols.size() - 1; i >= 0; --i) {
        auto note = std::dynamic_pointer_cast<Note>(m_symbols[i]);
        if (!note) {
            continue;
        }

        const bool inSlotRange = std::abs(note->xSlot() - xSlot) <= slotTolerance;
        const bool inStaffIndex = note->staffIndex() == staffIndex;
        const bool inStepRange = std::abs(note->staffStep() - staffStep) <= stepTolerance;
        if (inSlotRange && inStaffIndex && inStepRange) {
            return note;
        }
    }

    return {};
}

std::shared_ptr<Note> Score::noteAt(int xSlot, int staffStep, int slotTolerance, int stepTolerance) {
    return noteAt(xSlot, 0, staffStep, slotTolerance, stepTolerance);
}

std::shared_ptr<BarLine> Score::barLineAt(int xSlot, int staffIndex, int slotTolerance) {
    for (int i = m_symbols.size() - 1; i >= 0; --i) {
        auto barLine = std::dynamic_pointer_cast<BarLine>(m_symbols[i]);
        if (!barLine) {
            continue;
        }

        const bool inSlotRange = std::abs(barLine->xSlot() - xSlot) <= slotTolerance;
        const bool inStaffIndex = barLine->staffIndex() == staffIndex;
        if (inSlotRange && inStaffIndex) {
            return barLine;
        }
    }
    return {};
}

std::shared_ptr<BarLine> Score::barLineAt(int xSlot, int slotTolerance) {
    return barLineAt(xSlot, 0, slotTolerance);
}

std::shared_ptr<TimeSignature> Score::timeSignatureAt(int xSlot, int staffIndex, int slotTolerance) {
    for (int i = m_symbols.size() - 1; i >= 0; --i) {
        auto ts = std::dynamic_pointer_cast<TimeSignature>(m_symbols[i]);
        if (!ts) {
            continue;
        }

        const bool inSlotRange = std::abs(ts->xSlot() - xSlot) <= slotTolerance;
        const bool inStaffIndex = ts->staffIndex() == staffIndex;
        if (inSlotRange && inStaffIndex) {
            return ts;
        }
    }

    return {};
}

std::shared_ptr<TimeSignature> Score::timeSignatureAt(int xSlot, int slotTolerance) {
    return timeSignatureAt(xSlot, 0, slotTolerance);
}

std::shared_ptr<GlyphSymbol> Score::glyphAt(int xSlot, int staffIndex, int staffStep, int slotTolerance, int stepTolerance) {
    for (int i = m_symbols.size() - 1; i >= 0; --i) {
        auto glyph = std::dynamic_pointer_cast<GlyphSymbol>(m_symbols[i]);
        if (!glyph) {
            continue;
        }
        const auto type = glyph->type();
        if (type != MusicSymbol::SymbolType::TrebleClef
            && type != MusicSymbol::SymbolType::Rest
            && type != MusicSymbol::SymbolType::Sharp
            && type != MusicSymbol::SymbolType::Flat
            && type != MusicSymbol::SymbolType::Beccare) {
            continue;
        }

        const bool inSlotRange = std::abs(glyph->xSlot() - xSlot) <= slotTolerance;
        const bool inStaffIndex = glyph->staffIndex() == staffIndex;
        const bool inStepRange = std::abs(glyph->staffStep() - staffStep) <= stepTolerance;
        if (inSlotRange && inStaffIndex && inStepRange) {
            return glyph;
        }
    }
    return {};
}

std::shared_ptr<GlyphSymbol> Score::glyphAt(int xSlot, int staffStep, int slotTolerance, int stepTolerance) {
    return glyphAt(xSlot, 0, staffStep, slotTolerance, stepTolerance);
}

const QVector<std::shared_ptr<MusicSymbol>>& Score::symbols() const {
    return m_symbols;
}
