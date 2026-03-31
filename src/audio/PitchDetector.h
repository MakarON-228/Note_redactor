#pragma once

#include <vector>
#include <complex>

class PitchDetector {
public:
    PitchDetector(int sampleRate, int bufferSize);
    ~PitchDetector();

    // Returns the detected frequency in Hz, or 0 if no clear pitch is found
    float detectPitch(const std::vector<float>& audioData);

    // Converts frequency to MIDI note number (69 = A4 = 440Hz)
    static int frequencyToMidiNote(float frequency);

private:
    int m_sampleRate;
    int m_bufferSize;
    std::vector<float> m_window;
    
    // YIN algorithm implementation details
    std::vector<float> m_yinBuffer;
    int absoluteThreshold(float threshold);
    float parabolicInterpolation(int tauEstimate);
};
