#pragma once

#include <QMainWindow>
#include <memory>

class StaffWidget;
class AudioRecorder;
class QPushButton;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void toggleRecording();
    void onNoteDetected(int midiNote);

private:
    StaffWidget* m_staffWidget;
    std::unique_ptr<AudioRecorder> m_audioRecorder;
    QPushButton* m_recordButton;
};
