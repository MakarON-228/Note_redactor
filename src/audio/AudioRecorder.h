#pragma once

#include <QObject>
#include <QAudioSource>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QIODevice>
#include <QTimer>
#include <vector>
#include <memory>

class PitchDetector;

class AudioRecorder : public QObject {
    Q_OBJECT

public:
    explicit AudioRecorder(QObject* parent = nullptr);
    ~AudioRecorder();

    void startRecording();
    void stopRecording();
    bool isRecording() const;

signals:
    void noteDetected(int midiNote);

private slots:
    void processAudioData();

private:
    std::unique_ptr<QAudioSource> m_audioSource;
    QIODevice* m_audioDevice;
    std::unique_ptr<PitchDetector> m_pitchDetector;
    QTimer m_processTimer;
    
    int m_sampleRate;
    int m_bufferSize;
    std::vector<float> m_audioBuffer;
    
    int m_lastDetectedNote;
    int m_consecutiveDetections;
};
