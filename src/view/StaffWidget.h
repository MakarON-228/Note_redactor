#pragma once

#include "../common/ToolType.h"
#include "../model/Score.h"

#include <QWidget>
#include <QByteArray>
#include <QString>
#include <memory>

class StaffWidget : public QWidget {
    Q_OBJECT

public:
    explicit StaffWidget(QWidget* parent = nullptr);

    QSize minimumSizeHint() const override;

    // Exports the staff area to a file (PNG/PDF).
    bool exportToPng(const QString& filePath);
    bool exportToPdf(const QString& filePath);

    // Serializes current staff content (score + staff count) to JSON.
    QByteArray toJson() const;
    bool fromJson(const QByteArray& jsonData);

public slots:
    void setTool(ToolType tool);
    void addStaffRow();
    void moveSelectedNoteUp();
    void moveSelectedNoteDown();
    void moveSelectedNoteLeft();
    void moveSelectedNoteRight();
    void deleteSelectedNote();
    void addNoteFromMidi(int midiNote);
    void setSpacingCoefficient(int k);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    int staffTopY(int staffIndex) const;
    int staffBottomY(int staffIndex) const;
    int staffStepToY(int staffIndex, int step) const;
    int yToStaffIndex(int y) const;
    int yToStaffStep(int staffIndex, int y) const;
    int slotToX(int slot) const;
    int xToSlot(int x) const;

    void render(QPainter& painter, int renderWidth, int renderHeight);
    void drawStaff(QPainter& painter, int renderWidth);
    void drawSymbols(QPainter& painter);
    void drawLedgerLinesForStep(QPainter& painter, int x, int staffIndex, int step);

    ToolType m_currentTool;
    Score m_score;
    std::shared_ptr<MusicSymbol> m_selectedSymbol;
    std::shared_ptr<Note> m_selectedNote;
    int m_staffCount;
    bool m_dragging;
    int m_spacingK;
};
