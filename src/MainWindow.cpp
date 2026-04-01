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
#include <QFrame>
#include <QStyle>
#include <QSpinBox>
#include <QLabel>

static QPushButton* makeCtrlButton(const QString& icon, const QString& text, QWidget* parent) {
    auto* btn = new QPushButton(text, parent);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(
        "QPushButton {"
        "  background: #ffffff;"
        "  color: #2c3e50;"
        "  border: 1px solid #dcdfe6;"
        "  border-radius: 6px;"
        "  padding: 8px 14px;"
        "  font-size: 12px;"
        "  font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "  background: #ecf5ff;"
        "  border-color: #409eff;"
        "  color: #409eff;"
        "}"
        "QPushButton:pressed {"
        "  background: #d9ecff;"
        "  border-color: #337ecc;"
        "}"
    );
    return btn;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_staffWidget(new StaffWidget()),
      m_audioRecorder(std::make_unique<AudioRecorder>(this)) {
    auto* central = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto* palette = new SymbolPaletteWidget(central);
    auto* scrollArea = new QScrollArea(central);
    scrollArea->setWidget(m_staffWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: #f8f9fa; }");

    // Bottom control bar
    auto* controlsFrame = new QFrame(central);
    controlsFrame->setFrameStyle(QFrame::StyledPanel);
    controlsFrame->setFixedHeight(56);
    controlsFrame->setStyleSheet(
        "QFrame {"
        "  background: #ffffff;"
        "  border-top: 1px solid #e4e7ed;"
        "}"
    );
    auto* controlsRow = new QHBoxLayout(controlsFrame);
    controlsRow->setContentsMargins(16, 8, 16, 8);
    controlsRow->setSpacing(8);

    auto* upButton = makeCtrlButton("↑", "Move Up", controlsFrame);
    auto* downButton = makeCtrlButton("↓", "Move Down", controlsFrame);
    auto* deleteButton = makeCtrlButton("✕", "Delete", controlsFrame);
    auto* addStaffButton = makeCtrlButton("+", "Add Staff", controlsFrame);
    
    controlsRow->addWidget(upButton);
    controlsRow->addWidget(downButton);
    controlsRow->addWidget(deleteButton);
    controlsRow->addWidget(addStaffButton);
    controlsRow->addSpacing(16);

    auto* exportPngButton = makeCtrlButton("🖼", "Export PNG", controlsFrame);
    auto* exportPdfButton = makeCtrlButton("📄", "Export PDF", controlsFrame);
    auto* saveJsonButton = makeCtrlButton("💾", "Save", controlsFrame);
    auto* loadJsonButton = makeCtrlButton("📂", "Load", controlsFrame);
    
    controlsRow->addWidget(exportPngButton);
    controlsRow->addWidget(exportPdfButton);
    controlsRow->addWidget(saveJsonButton);
    controlsRow->addWidget(loadJsonButton);
    controlsRow->addStretch(1);

    // Spacing control group
    auto* spacingLayout = new QHBoxLayout();
    spacingLayout->setContentsMargins(0, 0, 0, 0);
    spacingLayout->setSpacing(4);

    auto* spacingLabel = new QLabel("Staff spacing:", controlsFrame);
    spacingLabel->setStyleSheet("color: #2c3e50; font-size: 12px; font-weight: 600;");

    auto* spacingMinusBtn = new QPushButton("−", controlsFrame);
    spacingMinusBtn->setFixedSize(28, 28);
    spacingMinusBtn->setCursor(Qt::PointingHandCursor);
    spacingMinusBtn->setStyleSheet(
        "QPushButton {"
        "  background: #ffffff;"
        "  color: #2c3e50;"
        "  border: 1px solid #b0bec5;"
        "  border-radius: 4px;"
        "  font-size: 16px;"
        "  font-weight: 700;"
        "}"
        "QPushButton:hover { background: #ecf5ff; border-color: #409eff; color: #409eff; }"
        "QPushButton:pressed { background: #d9ecff; }"
    );

    auto* spacingValueLabel = new QLabel("0", controlsFrame);
    m_spacingValueLabel = spacingValueLabel;
    spacingValueLabel->setStyleSheet("color: #2c3e50; font-size: 14px; font-weight: 600; min-width: 24px;");
    spacingValueLabel->setAlignment(Qt::AlignCenter);

    auto* spacingPlusBtn = new QPushButton("+", controlsFrame);
    spacingPlusBtn->setFixedSize(28, 28);
    spacingPlusBtn->setCursor(Qt::PointingHandCursor);
    spacingPlusBtn->setStyleSheet(
        "QPushButton {"
        "  background: #ffffff;"
        "  color: #2c3e50;"
        "  border: 1px solid #b0bec5;"
        "  border-radius: 4px;"
        "  font-size: 16px;"
        "  font-weight: 700;"
        "}"
        "QPushButton:hover { background: #ecf5ff; border-color: #409eff; color: #409eff; }"
        "QPushButton:pressed { background: #d9ecff; }"
    );

    connect(spacingMinusBtn, &QPushButton::clicked, this, &MainWindow::onSpacingMinus);
    connect(spacingPlusBtn, &QPushButton::clicked, this, &MainWindow::onSpacingPlus);

    spacingLayout->addWidget(spacingLabel);
    spacingLayout->addWidget(spacingMinusBtn);
    spacingLayout->addWidget(spacingValueLabel);
    spacingLayout->addWidget(spacingPlusBtn);
    controlsRow->addLayout(spacingLayout);
    controlsRow->addSpacing(16);

    m_recordButton = new QPushButton("●  Start Recording", controlsFrame);
    m_recordButton->setCursor(Qt::PointingHandCursor);
    m_recordButton->setFixedHeight(36);
    m_recordButton->setStyleSheet(
        "QPushButton {"
        "  background: #f56c6c;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 18px;"
        "  padding: 0 20px;"
        "  font-size: 13px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "  background: #f78989;"
        "}"
        "QPushButton:pressed {"
        "  background: #e04e4e;"
        "}"
    );
    controlsRow->addWidget(m_recordButton);

    rootLayout->addWidget(palette);
    rootLayout->addWidget(scrollArea, 1);
    rootLayout->addWidget(controlsFrame);

    setCentralWidget(central);
    setWindowTitle("Note Redactor");
    resize(1400, 500);
    setFixedWidth(1400);

    // Apply global stylesheet
    setStyleSheet(
        "QMainWindow {"
        "  background: #f8f9fa;"
        "}"
    );

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

void MainWindow::onSpacingMinus() {
    m_spacingK = qBound(-4, m_spacingK - 1, 10);
    m_spacingValueLabel->setText(QString::number(m_spacingK));
    m_staffWidget->setSpacingCoefficient(m_spacingK);
}

void MainWindow::onSpacingPlus() {
    m_spacingK = qBound(-4, m_spacingK + 1, 10);
    m_spacingValueLabel->setText(QString::number(m_spacingK));
    m_staffWidget->setSpacingCoefficient(m_spacingK);
}
