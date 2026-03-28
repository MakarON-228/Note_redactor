#pragma once

#include "MusicSymbol.h"
#include "BarLine.h"
#include "GlyphSymbol.h"
#include "Note.h"
#include "TimeSignature.h"
#include "Dot.h"

#include <QVector>
#include <memory>

class Score {
public:
    Score() = default;

    std::shared_ptr<Note> addNote(int xSlot, int staffIndex, int staffStep);
    std::shared_ptr<Note> addNote(int xSlot, int staffStep);
    std::shared_ptr<Note> addNote(int xSlot, int staffIndex, int staffStep, Note::Duration duration);
    std::shared_ptr<Note> addNote(int xSlot, int staffStep, Note::Duration duration);

    std::shared_ptr<BarLine> addBarLine(int xSlot, int staffIndex);
    std::shared_ptr<BarLine> addBarLine(int xSlot);

    std::shared_ptr<Dot> addDot(int xSlot, int staffIndex, int staffStep);
    std::shared_ptr<Dot> addDot(int xSlot, int staffStep);

    std::shared_ptr<GlyphSymbol> addGlyph(MusicSymbol::SymbolType type, int xSlot, int staffIndex, int staffStep);
    std::shared_ptr<GlyphSymbol> addGlyph(MusicSymbol::SymbolType type, int xSlot, int staffStep);
    std::shared_ptr<GlyphSymbol> addGlyph(MusicSymbol::SymbolType type, int xSlot, int staffIndex, int staffStep, Note::Duration restDuration);
    std::shared_ptr<GlyphSymbol> addGlyph(MusicSymbol::SymbolType type, int xSlot, int staffStep, Note::Duration restDuration);

    std::shared_ptr<TimeSignature> addTimeSignature(int xSlot, int staffIndex, int numerator, int denominator);
    std::shared_ptr<TimeSignature> addTimeSignature(int xSlot, int numerator, int denominator);

    bool removeNoteAt(int xSlot, int staffIndex, int staffStep, int slotTolerance = 0, int stepTolerance = 0);
    bool removeNoteAt(int xSlot, int staffStep, int slotTolerance = 0, int stepTolerance = 0);

    bool removeBarLineAt(int xSlot, int staffIndex, int slotTolerance = 0);
    bool removeBarLineAt(int xSlot, int slotTolerance = 0);

    bool removeDotAt(int xSlot, int staffIndex, int staffStep, int slotTolerance = 0, int stepTolerance = 0);
    bool removeDotAt(int xSlot, int staffStep, int slotTolerance = 0, int stepTolerance = 0);

    bool removeTimeSignatureAt(int xSlot, int staffIndex, int slotTolerance = 0);
    bool removeTimeSignatureAt(int xSlot, int slotTolerance = 0);
    bool removeSymbol(const std::shared_ptr<MusicSymbol>& symbol);

    std::shared_ptr<Note> noteAt(int xSlot, int staffIndex, int staffStep, int slotTolerance = 0, int stepTolerance = 0);
    std::shared_ptr<Note> noteAt(int xSlot, int staffStep, int slotTolerance = 0, int stepTolerance = 0);

    std::shared_ptr<BarLine> barLineAt(int xSlot, int staffIndex, int slotTolerance = 0);
    std::shared_ptr<BarLine> barLineAt(int xSlot, int slotTolerance = 0);

    std::shared_ptr<Dot> dotAt(int xSlot, int staffIndex, int staffStep, int slotTolerance = 0, int stepTolerance = 0);
    std::shared_ptr<Dot> dotAt(int xSlot, int staffStep, int slotTolerance = 0, int stepTolerance = 0);

    std::shared_ptr<TimeSignature> timeSignatureAt(int xSlot, int staffIndex, int slotTolerance = 0);
    std::shared_ptr<TimeSignature> timeSignatureAt(int xSlot, int slotTolerance = 0);

    std::shared_ptr<GlyphSymbol> glyphAt(int xSlot, int staffIndex, int staffStep, int slotTolerance = 0, int stepTolerance = 0);
    std::shared_ptr<GlyphSymbol> glyphAt(int xSlot, int staffStep, int slotTolerance = 0, int stepTolerance = 0);
    const QVector<std::shared_ptr<MusicSymbol>>& symbols() const;

private:
    QVector<std::shared_ptr<MusicSymbol>> m_symbols;
};
