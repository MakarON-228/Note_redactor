#include "MainWindow.h"

#include "view/StaffWidget.h"
#include "view/SymbolPaletteWidget.h"
#include "audio/AudioRecorder.h"

#include <QHBoxLayout>
#include <QFile>
#include <QFileDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QScrollArea>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_staffWidget(new StaffWidget()),
      m_audioRecorder(std::make_unique<AudioRecorder>(this)) {
    auto* central = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(8);

    auto* palette = new SymbolPaletteWidget(central);
    auto* scrollArea = new QScrollArea(central);
    scrollArea->setWidget(m_staffWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    auto* controlsRow = new QHBoxLayout();
    auto* upButton = new QPushButton("Move up", central);
    auto* downButton = new QPushButton("Move down", central);
    auto* deleteButton = new QPushButton("Delete selected", central);
    auto* addStaffButton = new QPushButton("Add staff", central);
    auto* exportPngButton = new QPushButton("Export PNG", central);
    auto* exportPdfButton = new QPushButton("Export PDF", central);
    auto* saveJsonButton = new QPushButton("Save JSON", central);
    auto* loadJsonButton = new QPushButton("Load JSON", central);
    m_recordButton = new QPushButton("Start Recording", central);
    
    controlsRow->addWidget(upButton);
    controlsRow->addWidget(downButton);
    controlsRow->addWidget(deleteButton);
    controlsRow->addWidget(addStaffButton);
    controlsRow->addWidget(exportPngButton);
    controlsRow->addWidget(exportPdfButton);
    controlsRow->addWidget(saveJsonButton);
    controlsRow->addWidget(loadJsonButton);
    controlsRow->addWidget(m_recordButton);
    controlsRow->addStretch(1);

    rootLayout->addWidget(palette);
    rootLayout->addWidget(scrollArea, 1);
    rootLayout->addLayout(controlsRow);

    setCentralWidget(central);
    setWindowTitle("Note redactor - Qt MVP");
    resize(1400, 500);
    setFixedWidth(1400);

    connect(palette, &SymbolPaletteWidget::toolChanged, m_staffWidget, &StaffWidget::setTool);
    connect(upButton, &QPushButton::clicked, m_staffWidget, &StaffWidget::moveSelectedNoteUp);
    connect(downButton, &QPushButton::clicked, m_staffWidget, &StaffWidget::moveSelectedNoteDown);
    connect(deleteButton, &QPushButton::clicked, m_staffWidget, &StaffWidget::deleteSelectedNote);
    connect(addStaffButton, &QPushButton::clicked, m_staffWidget, &StaffWidget::addStaffRow);

    connect(exportPngButton, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getSaveFileName(this, "Export PNG", "", "PNG Images (*.png)");
        if (!filePath.isEmpty()) {
            m_staffWidget->exportToPng(filePath);
        }
    });

    connect(exportPdfButton, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getSaveFileName(this, "Export PDF", "", "PDF Documents (*.pdf)");
        if (!filePath.isEmpty()) {
            m_staffWidget->exportToPdf(filePath);
        }
    });

    connect(saveJsonButton, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getSaveFileName(this, "Save JSON", "", "JSON Files (*.json)");
        if (!filePath.isEmpty()) {
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(m_staffWidget->toJson());
            }
        }
    });

    connect(loadJsonButton, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this, "Load JSON", "", "JSON Files (*.json)");
        if (!filePath.isEmpty()) {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly)) {
                m_staffWidget->fromJson(file.readAll());
            }
        }
    });

    connect(m_recordButton, &QPushButton::clicked, this, &MainWindow::toggleRecording);
    connect(m_audioRecorder.get(), &AudioRecorder::noteDetected, this, &MainWindow::onNoteDetected);
}

MainWindow::~MainWindow() = default;

void MainWindow::toggleRecording() {
    if (m_audioRecorder->isRecording()) {
        m_audioRecorder->stopRecording();
        m_recordButton->setText("Start Recording");
    } else {
        m_audioRecorder->startRecording();
        m_recordButton->setText("Stop Recording");
    }
}

void MainWindow::onNoteDetected(int midiNote) {
    m_staffWidget->addNoteFromMidi(midiNote);
}
