#pragma once

#include "MusicSymbol.h"

#include <QString>

class Note : public MusicSymbol {
public:
    enum class PitchClass {
        C = 0,
        D,
        E,
        F,
        G,
        A,
        B
    };

    enum class Accidental {
        Flat = -1,
        Natural = 0,
        Sharp = 1
    };

    enum class Duration {
        Whole,
        Half,
        Quarter,
        Eighth,
        Sixteenth
    };

    Note(int xSlot, int staffIndex, int staffStep, Duration duration);
    Note(int xSlot, int staffIndex, int staffStep);
    // Convenience overload: staffIndex defaults to 0.
    Note(int xSlot, int staffStep, Duration duration);
    // Backward-friendly ctor: defaults to first staff row.
    Note(int xSlot, int staffStep);

    int xSlot() const;
    int staffIndex() const;
    int staffStep() const;

    void setXSlot(int slot);
    void setStaffIndex(int staffIndex);
    void setStaffStep(int step);

    void moveHorizontal(int deltaSlots);
    void moveVertical(int deltaSteps);

    bool isOnLine() const;
    bool isBetweenLines() const;

    PitchClass pitchClass() const;
    int octave() const;
    Accidental accidental() const;
    Duration duration() const;
    void setDuration(Duration duration);

    QString noteName() const;
    double frequencyHz() const;

private:
    void updatePitchFromStaffStep();
    double calculateFrequency() const;

    int m_xSlot;
    int m_staffIndex;
    int m_staffStep;
    PitchClass m_pitchClass;
    int m_octave;
    Accidental m_accidental;
    Duration m_duration;
    double m_frequencyHz;
};
