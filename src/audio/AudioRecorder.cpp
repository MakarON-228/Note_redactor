#include "AudioRecorder.h"
#include "PitchDetector.h"
#include <QDebug>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QAudioDevice>

AudioRecorder::AudioRecorder(QObject* parent)
    : QObject(parent),
      m_audioDevice(nullptr),
      m_sampleRate(44100),
      m_bufferSize(4096),
      m_lastDetectedNote(-1),
      m_consecutiveDetections(0) {
      
    m_pitchDetector = std::make_unique<PitchDetector>(m_sampleRate, m_bufferSize);
    
    QAudioFormat format;
    format.setSampleRate(m_sampleRate);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Float);

    QAudioDevice info = QMediaDevices::defaultAudioInput();
    if (!info.isFormatSupported(format)) {
        qWarning() << "Default format not supported, trying to use nearest.";
    }

    m_audioSource = std::make_unique<QAudioSource>(info, format, this);
    
    connect(&m_processTimer, &QTimer::timeout, this, &AudioRecorder::processAudioData);
}

AudioRecorder::~AudioRecorder() {
    stopRecording();
}

void AudioRecorder::startRecording() {
    if (isRecording()) return;
    
    m_audioDevice = m_audioSource->start();
    m_processTimer.start(50); // Process every 50ms
    m_lastDetectedNote = -1;
    m_consecutiveDetections = 0;
}

void AudioRecorder::stopRecording() {
    if (!isRecording()) return;
    
    m_processTimer.stop();
    m_audioSource->stop();
    m_audioDevice = nullptr;
}

bool AudioRecorder::isRecording() const {
    return m_audioSource->state() == QAudio::ActiveState;
}

void AudioRecorder::processAudioData() {
    if (!m_audioDevice) return;

    qint64 bytesAvailable = m_audioSource->bytesAvailable();
    if (bytesAvailable < m_bufferSize * sizeof(float)) return;

    QByteArray data = m_audioDevice->read(m_bufferSize * sizeof(float));
    const float* floatData = reinterpret_cast<const float*>(data.constData());
    int numSamples = data.size() / sizeof(float);

    m_audioBuffer.assign(floatData, floatData + numSamples);

    float frequency = m_pitchDetector->detectPitch(m_audioBuffer);
    
    if (frequency > 0) {
        int midiNote = PitchDetector::frequencyToMidiNote(frequency);
        
        // Simple debouncing/smoothing
        if (midiNote == m_lastDetectedNote) {
            m_consecutiveDetections++;
            if (m_consecutiveDetections == 3) { // Require 3 consecutive detections
                emit noteDetected(midiNote);
            }
        } else {
            m_lastDetectedNote = midiNote;
            m_consecutiveDetections = 1;
        }
    } else {
        m_lastDetectedNote = -1;
        m_consecutiveDetections = 0;
    }
}
