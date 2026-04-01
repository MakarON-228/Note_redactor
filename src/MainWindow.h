#pragma once

#include <QMainWindow>
#include <memory>

class StaffWidget;
class AudioRecorder;
class QPushButton;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void toggleRecording();
    void onNoteDetected(int midiNote);
    void onSpacingMinus();
    void onSpacingPlus();

private:
    StaffWidget* m_staffWidget;
    std::unique_ptr<AudioRecorder> m_audioRecorder;
    QPushButton* m_recordButton;
    int m_spacingK = 0;
    QLabel* m_spacingValueLabel = nullptr;
};
