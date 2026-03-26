#include "Note.h"

#include <QtGlobal>
#include <array>
#include <cmath>

namespace {
constexpr int kBottomLineDiatonicFromC0 = 30; // E4 for treble clef.
}

Note::Note(int xSlot, int staffIndex, int staffStep, Duration duration)
    : MusicSymbol(SymbolType::Note),
      m_xSlot(xSlot),
      m_staffIndex(staffIndex),
      m_staffStep(staffStep),
      m_pitchClass(PitchClass::E),
      m_octave(4),
      m_accidental(Accidental::Natural),
      m_duration(duration),
      m_frequencyHz(0.0) {
    updatePitchFromStaffStep();
}

Note::Note(int xSlot, int staffIndex, int staffStep)
    : Note(xSlot, staffIndex, staffStep, Duration::Whole) {}

Note::Note(int xSlot, int staffStep, Duration duration)
    : Note(xSlot, 0, staffStep, duration) {}

Note::Note(int xSlot, int staffStep)
    : Note(xSlot, 0, staffStep, Duration::Whole) {}

int Note::xSlot() const {
    return m_xSlot;
}

int Note::staffIndex() const {
    return m_staffIndex;
}

int Note::staffStep() const {
    return m_staffStep;
}

void Note::setXSlot(int slot) {
    m_xSlot = qMax(0, slot);
}

void Note::setStaffIndex(int staffIndex) {
    m_staffIndex = (staffIndex < 0) ? 0 : staffIndex;
}

void Note::setStaffStep(int step) {
    m_staffStep = step;
    updatePitchFromStaffStep();
}

void Note::moveHorizontal(int deltaSlots) {
    setXSlot(m_xSlot + deltaSlots);
}

void Note::moveVertical(int deltaSteps) {
    setStaffStep(m_staffStep + deltaSteps);
}

bool Note::isOnLine() const {
    return (m_staffStep % 2) == 0;
}

bool Note::isBetweenLines() const {
    return !isOnLine();
}

Note::PitchClass Note::pitchClass() const {
    return m_pitchClass;
}

int Note::octave() const {
    return m_octave;
}

Note::Accidental Note::accidental() const {
    return m_accidental;
}

Note::Duration Note::duration() const {
    return m_duration;
}

void Note::setDuration(Duration duration) {
    m_duration = duration;
}

QString Note::noteName() const {
    static const std::array<QString, 7> names{
        "C", "D", "E", "F", "G", "A", "B"
    };
    QString accidentalText;
    if (m_accidental == Accidental::Sharp) {
        accidentalText = "#";
    } else if (m_accidental == Accidental::Flat) {
        accidentalText = "b";
    }

    return names[static_cast<int>(m_pitchClass)] + accidentalText + QString::number(m_octave);
}

double Note::frequencyHz() const {
    return m_frequencyHz;
}

void Note::updatePitchFromStaffStep() {
    // Every staff step is one diatonic step in treble clef from E4 upward/downward.
    const int absoluteDiatonic = kBottomLineDiatonicFromC0 + m_staffStep;
    int octave = absoluteDiatonic / 7;
    int pitchIndex = absoluteDiatonic % 7;

    if (pitchIndex < 0) {
        pitchIndex += 7;
        octave -= 1;
    }

    m_octave = octave;
    m_pitchClass = static_cast<PitchClass>(pitchIndex);
    m_frequencyHz = calculateFrequency();
}

double Note::calculateFrequency() const {
    static const std::array<int, 7> semitoneFromC{
        0, 2, 4, 5, 7, 9, 11
    };

    const int accidentalOffset = static_cast<int>(m_accidental);
    const int midiNote = (m_octave + 1) * 12
        + semitoneFromC[static_cast<int>(m_pitchClass)]
        + accidentalOffset;
    return 440.0 * std::pow(2.0, (midiNote - 69) / 12.0);
}
