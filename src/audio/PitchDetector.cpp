#include "PitchDetector.h"
#include <cmath>
#include <algorithm>

PitchDetector::PitchDetector(int sampleRate, int bufferSize)
    : m_sampleRate(sampleRate), m_bufferSize(bufferSize) {
    m_yinBuffer.resize(bufferSize / 2);
}

PitchDetector::~PitchDetector() = default;

float PitchDetector::detectPitch(const std::vector<float>& audioData) {
    if (audioData.size() < m_bufferSize) return 0.0f;

    int halfBufferSize = m_bufferSize / 2;
    
    // Step 1: Calculate difference function
    for (int tau = 0; tau < halfBufferSize; tau++) {
        m_yinBuffer[tau] = 0;
    }
    for (int tau = 1; tau < halfBufferSize; tau++) {
        for (int i = 0; i < halfBufferSize; i++) {
            float delta = audioData[i] - audioData[i + tau];
            m_yinBuffer[tau] += delta * delta;
        }
    }

    // Step 2: Cumulative mean normalized difference function
    m_yinBuffer[0] = 1;
    float runningSum = 0;
    for (int tau = 1; tau < halfBufferSize; tau++) {
        runningSum += m_yinBuffer[tau];
        m_yinBuffer[tau] *= tau / runningSum;
    }

    // Step 3: Absolute threshold
    int tauEstimate = absoluteThreshold(0.1f); // 0.1 is a common threshold

    // Step 4: Parabolic interpolation
    if (tauEstimate != -1) {
        float betterTau = parabolicInterpolation(tauEstimate);
        return m_sampleRate / betterTau;
    }

    return 0.0f;
}

int PitchDetector::absoluteThreshold(float threshold) {
    int halfBufferSize = m_bufferSize / 2;
    for (int tau = 2; tau < halfBufferSize; tau++) {
        if (m_yinBuffer[tau] < threshold) {
            while (tau + 1 < halfBufferSize && m_yinBuffer[tau + 1] < m_yinBuffer[tau]) {
                tau++;
            }
            return tau;
        }
    }
    return -1;
}

float PitchDetector::parabolicInterpolation(int tauEstimate) {
    int halfBufferSize = m_bufferSize / 2;
    int x0 = (tauEstimate < 1) ? tauEstimate : tauEstimate - 1;
    int x2 = (tauEstimate + 1 < halfBufferSize) ? tauEstimate + 1 : tauEstimate;
    
    if (x0 == tauEstimate) {
        return (m_yinBuffer[tauEstimate] <= m_yinBuffer[x2]) ? tauEstimate : x2;
    }
    if (x2 == tauEstimate) {
        return (m_yinBuffer[tauEstimate] <= m_yinBuffer[x0]) ? tauEstimate : x0;
    }
    
    float s0 = m_yinBuffer[x0];
    float s1 = m_yinBuffer[tauEstimate];
    float s2 = m_yinBuffer[x2];
    
    return tauEstimate + 0.5f * (s2 - s0) / (2.0f * s1 - s2 - s0);
}

int PitchDetector::frequencyToMidiNote(float frequency) {
    if (frequency <= 0) return -1;
    return std::round(69 + 12 * std::log2(frequency / 440.0f));
}
