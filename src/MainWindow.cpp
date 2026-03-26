#include "MainWindow.h"

#include "view/StaffWidget.h"
#include "view/SymbolPaletteWidget.h"

#include <QHBoxLayout>
#include <QFile>
#include <QFileDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QScrollArea>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    auto* central = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(8);

    auto* palette = new SymbolPaletteWidget(central);
    auto* staff = new StaffWidget();
    auto* scrollArea = new QScrollArea(central);
    scrollArea->setWidget(staff);
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
    controlsRow->addWidget(upButton);
    controlsRow->addWidget(downButton);
    controlsRow->addWidget(deleteButton);
    controlsRow->addWidget(addStaffButton);
    controlsRow->addWidget(exportPngButton);
    controlsRow->addWidget(exportPdfButton);
    controlsRow->addWidget(saveJsonButton);
    controlsRow->addWidget(loadJsonButton);
    controlsRow->addStretch(1);

    rootLayout->addWidget(palette);
    rootLayout->addWidget(scrollArea, 1);
    rootLayout->addLayout(controlsRow);

    setCentralWidget(central);
    setWindowTitle("Note redactor - Qt MVP");
    resize(1400, 500);
    setFixedWidth(1400);

    connect(palette, &SymbolPaletteWidget::toolChanged, staff, &StaffWidget::setTool);
    connect(upButton, &QPushButton::clicked, staff, &StaffWidget::moveSelectedNoteUp);
    connect(downButton, &QPushButton::clicked, staff, &StaffWidget::moveSelectedNoteDown);
    connect(deleteButton, &QPushButton::clicked, staff, &StaffWidget::deleteSelectedNote);
    connect(addStaffButton, &QPushButton::clicked, staff, &StaffWidget::addStaffRow);

    connect(exportPngButton, &QPushButton::clicked, this, [staff]() {
        const QString path = QFileDialog::getSaveFileName(
            nullptr,
            "Export staff as PNG",
            "staff.png",
            "PNG files (*.png)"
        );
        if (path.isEmpty()) {
            return;
        }
        staff->exportToPng(path);
    });

    connect(exportPdfButton, &QPushButton::clicked, this, [staff]() {
        const QString path = QFileDialog::getSaveFileName(
            nullptr,
            "Export staff as PDF",
            "staff.pdf",
            "PDF files (*.pdf)"
        );
        if (path.isEmpty()) {
            return;
        }
        staff->exportToPdf(path);
    });

    connect(saveJsonButton, &QPushButton::clicked, this, [staff]() {
        const QString path = QFileDialog::getSaveFileName(
            nullptr,
            "Save score as JSON",
            "score.json",
            "JSON files (*.json)"
        );
        if (path.isEmpty()) {
            return;
        }

        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            return;
        }
        file.write(staff->toJson());
        file.close();
    });

    connect(loadJsonButton, &QPushButton::clicked, this, [staff]() {
        const QString path = QFileDialog::getOpenFileName(
            nullptr,
            "Load score from JSON",
            "",
            "JSON files (*.json)"
        );
        if (path.isEmpty()) {
            return;
        }

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return;
        }
        const QByteArray data = file.readAll();
        file.close();

        staff->fromJson(data);
    });
}
