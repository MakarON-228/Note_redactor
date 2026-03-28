#include "StaffWidget.h"

#include "../model/BarLine.h"
#include "../model/GlyphSymbol.h"
#include "../model/TimeSignature.h"
#include "../model/Dot.h"

#include <cmath>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QSvgRenderer>
#include <QInputDialog>
#include <QImage>
#include <QPdfWriter>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPageLayout>
#include <algorithm>
#include <QInputDialog>
#include <QFontMetricsF>
#include <ctime>
#include <memory>
#include <qnamespace.h>

namespace {
constexpr int kStaffFirstTopY = 90;
constexpr int kStaffLeftMargin = 70;
constexpr int kLineSpacing = 16;
constexpr int kHalfStepPx = kLineSpacing / 2;
constexpr int kStaffLinesCount = 5;
constexpr int kStaffRowPitch = 175; // расстояние между верхними линиями стана
constexpr int kSlotWidth = 28;
constexpr int kNoteWidth = 24;
constexpr int kNoteHeight = 12;
constexpr int kStaffTopStep = (kStaffLinesCount - 1) * 2; // 8
constexpr int kDefaultTrebleClefSlot = 0;
constexpr int kDefaultTrebleClefStaffStep = 3;
constexpr int kTimeSigBoxWidthPx = 40;
constexpr int kTimeSigFontSizeMinPt = 6;
constexpr int kTimeSigFontSizeMaxPt = 60;
constexpr int kTimeSigBarLenPx = 20;

double noteRectHForDuration(Note::Duration d) {
    switch (d) {
    case Note::Duration::Whole:
        return kNoteHeight;
    case Note::Duration::Half:
        return 60.0;
    case Note::Duration::Quarter:
        return 60.0;
    case Note::Duration::Eighth:
        return 60.0;
    case Note::Duration::Sixteenth:
        return 60.0;
    default:
        return kNoteHeight;
    }
}

double noteRectYFactor(Note::Duration d) {
    switch (d) {
    case Note::Duration::Whole:
        return 0.5;
    case Note::Duration::Half:
        return 0.85;
    case Note::Duration::Quarter:
        return 0.85;
    case Note::Duration::Eighth:
        return 0.85;
    case Note::Duration::Sixteenth:
        return 0.85;
    default:
        return 0.5;
    }
}

const char* svgResourceForNoteDuration(Note::Duration duration) {
    switch (duration) {
    case Note::Duration::Whole:
        return ":/assets/svg/whole_note.svg";
    case Note::Duration::Half:
        return ":/assets/svg/half_note.svg";
    case Note::Duration::Quarter:
        return ":/assets/svg/quarter_note.svg";
    case Note::Duration::Eighth:
        return ":/assets/svg/eighth_note.svg";
    case Note::Duration::Sixteenth:
        return ":/assets/svg/sixteenth_note.svg";
    default:
        return ":/assets/svg/whole_note.svg";
    }
}

const char* svgResourceForType(MusicSymbol::SymbolType type) {
    switch (type) {
    case MusicSymbol::SymbolType::TrebleClef:
        return ":/assets/svg/treble_clef.svg";
    case MusicSymbol::SymbolType::Sharp:
        return ":/assets/svg/sharp.svg";
    case MusicSymbol::SymbolType::Flat:
        return ":/assets/svg/flat.svg";
    case MusicSymbol::SymbolType::Beccare:
        return ":/assets/svg/beccare.svg";
    default:
        return nullptr;
    }
}

const char* svgResourceForRestDuration(Note::Duration duration) {
    switch (duration) {
    case Note::Duration::Whole:
        return ":/assets/svg/whole_rest.svg";
    case Note::Duration::Half:
        return ":/assets/svg/half_rest.svg";
    case Note::Duration::Quarter:
        return ":/assets/svg/quarter_rest.svg";
    case Note::Duration::Eighth:
        return ":/assets/svg/eighth_rest.svg";
    case Note::Duration::Sixteenth:
        return ":/assets/svg/sixteenth_rest.svg";
    default:
        return ":/assets/svg/whole_rest.svg";
    }
}
}

StaffWidget::StaffWidget(QWidget* parent)
    : QWidget(parent),
      m_currentTool(ToolType::Select),
      m_staffCount(1),
      m_dragging(false) {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    // Treble clef is always rendered at the beginning of each staff row.
    m_score.addGlyph(
        MusicSymbol::SymbolType::TrebleClef,
        kDefaultTrebleClefSlot,
        0,
        kDefaultTrebleClefStaffStep
    );
}

QSize StaffWidget::minimumSizeHint() const {
    const int firstBottom = kStaffFirstTopY + (kStaffLinesCount - 1) * kLineSpacing;
    const int lastTop = kStaffFirstTopY + (m_staffCount - 1) * kStaffRowPitch;
    const int lastBottom = lastTop + (kStaffLinesCount - 1) * kLineSpacing;
    const int paddingBottom = 120;
    return {1390, lastBottom + paddingBottom};
}

void StaffWidget::setTool(ToolType tool) {
    m_currentTool = tool;
}

void StaffWidget::addStaffRow() {
    const int newStaffIndex = m_staffCount;
    m_staffCount = qMax(1, m_staffCount + 1);
    m_score.addGlyph(
        MusicSymbol::SymbolType::TrebleClef,
        kDefaultTrebleClefSlot,
        newStaffIndex,
        kDefaultTrebleClefStaffStep
    );
    updateGeometry();
    resize(sizeHint());
    update();
}

void StaffWidget::moveSelectedNoteUp() {
    if (m_selectedNote) {
        m_selectedNote->moveVertical(1);
        update();
        return;
    }
    if (auto glyph = std::dynamic_pointer_cast<GlyphSymbol>(m_selectedSymbol)) {
        if (glyph->type() == MusicSymbol::SymbolType::TrebleClef) {
            return; // Clef is fixed and auto-inserted.
        }
        glyph->setStaffStep(glyph->staffStep() + 1);
        update();
    }
    if (auto dot = std::dynamic_pointer_cast<Dot>(m_selectedSymbol)) {
        dot->setStaffStep(dot->staffStep() + 1);
        update();
    }
}

void StaffWidget::moveSelectedNoteDown() {
    if (m_selectedNote) {
        m_selectedNote->moveVertical(-1);
        update();
        return;
    }
    if (auto glyph = std::dynamic_pointer_cast<GlyphSymbol>(m_selectedSymbol)) {
        if (glyph->type() == MusicSymbol::SymbolType::TrebleClef) {
            return; // Clef is fixed and auto-inserted.
        }
        glyph->setStaffStep(glyph->staffStep() - 1);
        update();
    }
    if (auto dot = std::dynamic_pointer_cast<Dot>(m_selectedSymbol)) {
        dot->setStaffStep(dot->staffStep() - 1);
        update();
    }
}

void StaffWidget::moveSelectedNoteLeft() {
    if (m_selectedNote) {
        m_selectedNote->moveHorizontal(-1);
        update();
        return;
    }
    if (auto glyph = std::dynamic_pointer_cast<GlyphSymbol>(m_selectedSymbol)) {
        if (glyph->type() == MusicSymbol::SymbolType::TrebleClef) {
            return; // Clef is fixed and auto-inserted.
        }
        glyph->setXSlot(glyph->xSlot() - 1);
        update();
        return;
    }
    if (auto barLine = std::dynamic_pointer_cast<BarLine>(m_selectedSymbol)) {
        barLine->setXSlot(barLine->xSlot() - 1);
        update();
        return;
    }
    if (auto dot = std::dynamic_pointer_cast<Dot>(m_selectedSymbol)) {
        dot->setXSlot(dot->xSlot() - 1);
        update();
        return;
    }
    if (auto timeSig = std::dynamic_pointer_cast<TimeSignature>(m_selectedSymbol)) {
        timeSig->setXSlot(timeSig->xSlot() - 1);
        update();
    }
}

void StaffWidget::moveSelectedNoteRight() {
    if (m_selectedNote) {
        m_selectedNote->moveHorizontal(1);
        update();
        return;
    }
    if (auto glyph = std::dynamic_pointer_cast<GlyphSymbol>(m_selectedSymbol)) {
        if (glyph->type() == MusicSymbol::SymbolType::TrebleClef) {
            return; // Clef is fixed and auto-inserted.
        }
        glyph->setXSlot(glyph->xSlot() + 1);
        update();
        return;
    }
    if (auto barLine = std::dynamic_pointer_cast<BarLine>(m_selectedSymbol)) {
        barLine->setXSlot(barLine->xSlot() + 1);
        update();
        return;
    }
    if (auto dot = std::dynamic_pointer_cast<Dot>(m_selectedSymbol)) {
        dot->setXSlot(dot->xSlot() + 1);
        update();
        return;
    }
    if (auto timeSig = std::dynamic_pointer_cast<TimeSignature>(m_selectedSymbol)) {
        timeSig->setXSlot(timeSig->xSlot() + 1);
        update();
    }
}

void StaffWidget::deleteSelectedNote() {
    if (m_selectedNote) {
        m_score.removeNoteAt(
            m_selectedNote->xSlot(),
            m_selectedNote->staffIndex(),
            m_selectedNote->staffStep(),
            0,
            0
        );
        m_selectedSymbol.reset();
        m_selectedNote.reset();
        update();
        return;
    }
    if (m_selectedSymbol) {
        if (m_selectedSymbol->type() == MusicSymbol::SymbolType::TrebleClef) {
            return; // Clef is fixed and auto-inserted.
        }
        m_score.removeSymbol(m_selectedSymbol);
        m_selectedSymbol.reset();
        update();
    }
}

void StaffWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    render(painter, width(), height());
}

void StaffWidget::render(QPainter& painter, int renderWidth, int renderHeight) {
    painter.fillRect(QRect(0, 0, renderWidth, renderHeight), QColor(252, 252, 252));
    painter.setRenderHint(QPainter::Antialiasing, true);

    drawStaff(painter, renderWidth);
    drawSymbols(painter);
}

void StaffWidget::mousePressEvent(QMouseEvent* event) {
    setFocus();

    const int slot = xToSlot(event->position().x());
    const int insertSlot = qMax(slot, kDefaultTrebleClefSlot + 1);
    const int staffIndex = yToStaffIndex(event->position().y());
    const int step = yToStaffStep(staffIndex, event->position().y());
    auto hitNote = m_score.noteAt(slot, staffIndex, step, 0, 1);
    auto hitGlyph = m_score.glyphAt(slot, staffIndex, step, 0, 1);
    auto hitBarLine = m_score.barLineAt(slot, staffIndex, 0);
    auto hitDot = m_score.dotAt(slot, staffIndex, step, 0, 1);
    auto hitTimeSignature = m_score.timeSignatureAt(slot, staffIndex, 1);

    switch (m_currentTool) {
    case ToolType::InsertWholeNote:
        m_selectedNote = m_score.addNote(insertSlot, staffIndex, step);
        m_selectedSymbol = m_selectedNote;
        update();
        break;
    case ToolType::InsertHalfNote:
        m_selectedNote = m_score.addNote(insertSlot, staffIndex, step, Note::Duration::Half);
        m_selectedSymbol = m_selectedNote;
        update();
        break;
    case ToolType::InsertQuarterNote:
        m_selectedNote = m_score.addNote(insertSlot, staffIndex, step, Note::Duration::Quarter);
        m_selectedSymbol = m_selectedNote;
        update();
        break;
    case ToolType::InsertEighthNote:
        m_selectedNote = m_score.addNote(insertSlot, staffIndex, step, Note::Duration::Eighth);
        m_selectedSymbol = m_selectedNote;
        update();
        break;
    case ToolType::InsertSixteenthNote:
        m_selectedNote = m_score.addNote(insertSlot, staffIndex, step, Note::Duration::Sixteenth);
        m_selectedSymbol = m_selectedNote;
        update();
        break;
    case ToolType::InsertBarLine:
        m_selectedSymbol = m_score.addBarLine(slot, staffIndex);
        m_selectedNote.reset();
        update();
        break;
    case ToolType::InsertDot:
        m_selectedSymbol = m_score.addDot(slot, staffIndex, step);
        m_selectedNote.reset();
        update();
        break;
    case ToolType::InsertTimeSignature: {
        const int numeratorDefault = 4;
        const int denominatorDefault = 4;
        bool okNum = false;
        bool okDen = false;

        const int numerator = QInputDialog::getInt(
            this,
            "Time signature",
            "Numerator:",
            numeratorDefault,
            1,
            99,
            1,
            &okNum
        );
        if (!okNum) {
            break;
        }

        const int denominator = QInputDialog::getInt(
            this,
            "Time signature",
            "Denominator:",
            denominatorDefault,
            1,
            99,
            1,
            &okDen
        );
        if (!okDen) {
            break;
        }

        m_selectedSymbol = m_score.addTimeSignature(
            insertSlot,
            staffIndex,
            numerator,
            denominator
        );
        m_selectedNote.reset();
        update();
        break;
    }
    case ToolType::InsertTrebleClef:
        // Treble clef is auto-inserted; ignore manual placement.
        m_selectedSymbol.reset();
        m_selectedNote.reset();
        update();
        break;
    case ToolType::InsertWholeRest:
        m_selectedSymbol = m_score.addGlyph(
            MusicSymbol::SymbolType::Rest,
            insertSlot,
            staffIndex,
            step,
            Note::Duration::Whole
        );
        m_selectedNote.reset();
        update();
        break;
    case ToolType::InsertHalfRest:
        m_selectedSymbol = m_score.addGlyph(
            MusicSymbol::SymbolType::Rest,
            insertSlot,
            staffIndex,
            step,
            Note::Duration::Half
        );
        m_selectedNote.reset();
        update();
        break;
    case ToolType::InsertQuarterRest:
        m_selectedSymbol = m_score.addGlyph(
            MusicSymbol::SymbolType::Rest,
            insertSlot,
            staffIndex,
            step,
            Note::Duration::Quarter
        );
        m_selectedNote.reset();
        update();
        break;
    case ToolType::InsertEighthRest:
        m_selectedSymbol = m_score.addGlyph(
            MusicSymbol::SymbolType::Rest,
            insertSlot,
            staffIndex,
            step,
            Note::Duration::Eighth
        );
        m_selectedNote.reset();
        update();
        break;
    case ToolType::InsertSixteenthRest:
        m_selectedSymbol = m_score.addGlyph(
            MusicSymbol::SymbolType::Rest,
            insertSlot,
            staffIndex,
            step,
            Note::Duration::Sixteenth
        );
        m_selectedNote.reset();
        update();
        break;
    case ToolType::InsertRest:
        // Legacy tool: keep old behavior but map to a default duration.
        m_selectedSymbol = m_score.addGlyph(
            MusicSymbol::SymbolType::Rest,
            insertSlot,
            staffIndex,
            step,
            Note::Duration::Quarter
        );
        m_selectedNote.reset();
        update();
        break;
    case ToolType::InsertSharp:
        m_selectedSymbol = m_score.addGlyph(MusicSymbol::SymbolType::Sharp, insertSlot, staffIndex, step);
        m_selectedNote.reset();
        update();
        break;
    case ToolType::InsertFlat:
        m_selectedSymbol = m_score.addGlyph(MusicSymbol::SymbolType::Flat, insertSlot, staffIndex, step);
        m_selectedNote.reset();
        update();
        break;
    case ToolType::InsertBeccare:
        m_selectedSymbol = m_score.addGlyph(MusicSymbol::SymbolType::Beccare, insertSlot, staffIndex, step);
        m_selectedNote.reset();
        update();
        break;
    
    case ToolType::Select:
        if (hitNote) {
            m_selectedSymbol = hitNote;
            m_selectedNote = hitNote;
        } else if (hitGlyph) {
            if (hitGlyph->type() == MusicSymbol::SymbolType::TrebleClef) {
                m_selectedSymbol.reset();
                m_selectedNote.reset();
            } else {
                m_selectedSymbol = hitGlyph;
                m_selectedNote.reset();
            }
        } else if (hitBarLine) {
            m_selectedSymbol = hitBarLine;
            m_selectedNote.reset();
        } else if (hitDot) {
            m_selectedSymbol = hitDot;
            m_selectedNote.reset();
        } else if (hitTimeSignature) {
            m_selectedSymbol = hitTimeSignature;
            m_selectedNote.reset();
        } else {
            m_selectedSymbol.reset();
            m_selectedNote.reset();
        }
        m_dragging = static_cast<bool>(m_selectedSymbol);
        update();
        break;
    case ToolType::Eraser:
        {
            bool removed = false;
            removed = removed || m_score.removeNoteAt(slot, staffIndex, step, 0, 1);
            removed = removed || m_score.removeBarLineAt(slot, staffIndex, 0);
            removed = removed || m_score.removeDotAt(slot, staffIndex, step, 0, 1);
            removed = removed || m_score.removeTimeSignatureAt(slot, staffIndex, 1);
            if (auto hit = m_score.glyphAt(slot, staffIndex, step, 0, 1)) {
                if (hit->type() != MusicSymbol::SymbolType::TrebleClef) {
                    m_score.removeSymbol(hit);
                    removed = true;
                }
            }
            if (removed) {
                m_selectedNote.reset();
                m_selectedSymbol.reset();
                update();
            }
        }
        break;
    default:
        // Placeholder tools for future symbol insertion via SVG.
        break;
    }

    QWidget::mousePressEvent(event);
}

void StaffWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!m_dragging || !m_selectedSymbol || !(event->buttons() & Qt::LeftButton)) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    const int newSlot = xToSlot(event->position().x());
    if (auto note = std::dynamic_pointer_cast<Note>(m_selectedSymbol)) {
        note->setXSlot(newSlot);
    } else if (auto glyph = std::dynamic_pointer_cast<GlyphSymbol>(m_selectedSymbol)) {
        glyph->setXSlot(newSlot);
        const int y = event->position().y();
        const int newStaffIndex = yToStaffIndex(y);
        glyph->setStaffIndex(newStaffIndex);
        glyph->setStaffStep(yToStaffStep(newStaffIndex, y));
    } else if (auto barLine = std::dynamic_pointer_cast<BarLine>(m_selectedSymbol)) {
        barLine->setXSlot(newSlot);
    } else if (auto dot = std::dynamic_pointer_cast<Dot>(m_selectedSymbol)) {
        dot->setXSlot(newSlot);
    } else if (auto timeSig = std::dynamic_pointer_cast<TimeSignature>(m_selectedSymbol)) {
        timeSig->setXSlot(newSlot);
    }
    update();
    QWidget::mouseMoveEvent(event);
}

void StaffWidget::mouseReleaseEvent(QMouseEvent* event) {
    m_dragging = false;
    QWidget::mouseReleaseEvent(event);
}

void StaffWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Up) {
        moveSelectedNoteUp();
        return;
    }
    if (event->key() == Qt::Key_Down) {
        moveSelectedNoteDown();
        return;
    }
    if (event->key() == Qt::Key_Left) {
        moveSelectedNoteLeft();
        return;
    }
    if (event->key() == Qt::Key_Right) {
        moveSelectedNoteRight();
        return;
    }
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        if (m_selectedNote) {
            deleteSelectedNote();
        } else if (m_selectedSymbol) {
            if (m_selectedSymbol->type() != MusicSymbol::SymbolType::TrebleClef) {
                m_score.removeSymbol(m_selectedSymbol);
                m_selectedSymbol.reset();
                update();
            }
        }
        return;
    }

    QWidget::keyPressEvent(event);
}

int StaffWidget::staffTopY(int staffIndex) const {
    return kStaffFirstTopY + staffIndex * kStaffRowPitch;
}

int StaffWidget::staffBottomY(int staffIndex) const {
    return staffTopY(staffIndex) + (kStaffLinesCount - 1) * kLineSpacing;
}

int StaffWidget::staffStepToY(int staffIndex, int step) const {
    return staffBottomY(staffIndex) - step * kHalfStepPx;
}

int StaffWidget::yToStaffIndex(int y) const {
    if (m_staffCount <= 1) {
        return 0;
    }

    const int staffCenterY0 = kStaffFirstTopY + 2 * kLineSpacing;
    const double exact = static_cast<double>(y - staffCenterY0) / kStaffRowPitch;
    const int idx = static_cast<int>(std::lround(exact));
    return qBound(0, idx, m_staffCount - 1);
}

int StaffWidget::yToStaffStep(int staffIndex, int y) const {
    const double step = static_cast<double>(staffBottomY(staffIndex) - y) / kHalfStepPx;
    return static_cast<int>(std::lround(step));
}

int StaffWidget::slotToX(int slot) const {
    return kStaffLeftMargin + slot * kSlotWidth;
}

int StaffWidget::xToSlot(int x) const {
    const int raw = static_cast<int>(std::lround(static_cast<double>(x - kStaffLeftMargin) / kSlotWidth));
    return qMax(0, raw);
}

void StaffWidget::drawStaff(QPainter& painter, int renderWidth) {
    painter.setPen(QPen(Qt::black, 2));
    const int widthRight = renderWidth - 30;
    for (int staff = 0; staff < m_staffCount; ++staff) {
        for (int line = 0; line < kStaffLinesCount; ++line) {
            const int y = staffTopY(staff) + line * kLineSpacing;
            painter.drawLine(kStaffLeftMargin - 40, y, widthRight, y);
        }
    }
}

void StaffWidget::drawSymbols(QPainter& painter) {
    for (const auto& symbol : m_score.symbols()) {
        auto note = std::dynamic_pointer_cast<Note>(symbol);
        if (note) {
            const int x = slotToX(note->xSlot());
            const int staffIndex = note->staffIndex();
            const int y = staffStepToY(staffIndex, note->staffStep());

            drawLedgerLinesForStep(painter, x, staffIndex, note->staffStep());
            const double rectH = noteRectHForDuration(note->duration());
            const double yFactor = noteRectYFactor(note->duration());
            const QRectF targetRect(
                x - kNoteWidth / 2.0,
                y - rectH * yFactor,
                kNoteWidth,
                rectH);
            const char* path = svgResourceForNoteDuration(note->duration());
            QSvgRenderer renderer(QString::fromUtf8(path));
            if (renderer.isValid()) {
                painter.save();
                painter.setClipRect(targetRect);
                renderer.render(&painter, targetRect);
                painter.restore();
            } else {
                painter.setPen(QPen(Qt::black, 2));
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(targetRect);
            }
            if (m_selectedSymbol == note) {
                painter.setPen(QPen(QColor(20, 90, 220), 2, Qt::DashLine));
                painter.setBrush(Qt::NoBrush);
                painter.drawRect(targetRect.adjusted(-2, -2, 2, 2));
            }
            continue;
        }

        auto glyph = std::dynamic_pointer_cast<GlyphSymbol>(symbol);
        if (glyph) {
            const int x = slotToX(glyph->xSlot());
            const int staffIndex = glyph->staffIndex();
            const int y = staffStepToY(staffIndex, glyph->staffStep());
            QRectF targetRect;

            switch (glyph->type()) {
            case MusicSymbol::SymbolType::TrebleClef:
                targetRect = QRectF(x - 12, y - 46, 24, 92);
                break;
            case MusicSymbol::SymbolType::Rest:
                switch (glyph->restDuration()) {
                case Note::Duration::Whole:
                    targetRect = QRectF(x - 9, y, 22, 10);
                    break;
                case Note::Duration::Half:
                    targetRect = QRectF(x - 9, y, 22, 10);
                    break;
                case Note::Duration::Quarter:
                    targetRect = QRectF(x - 9, y - 18, 18, 36);
                    break;
                case Note::Duration::Eighth:
                    targetRect = QRectF(x - 9, y - 18, 18, 36);
                    break;
                case Note::Duration::Sixteenth:
                    targetRect = QRectF(x - 9, y - 20, 18, 40);
                    break;
                default:
                    targetRect = QRectF(x - 9, y - 18, 18, 36);
                    break;
                }
                break;
            case MusicSymbol::SymbolType::Flat:
                targetRect = QRectF(x, y - 10, 15, 26);
                break;
            case MusicSymbol::SymbolType::Sharp:
                targetRect = QRectF(x, y - 5, 15, 26);
                break;
            case MusicSymbol::SymbolType::Beccare:
                targetRect = QRectF(x, y - 5, 15, 26);
                break;
            default:
                targetRect = QRectF(x, y - 10, 18, 36);
                break;
            }

            const char* path = nullptr;
            if (glyph->type() == MusicSymbol::SymbolType::Rest) {
                path = svgResourceForRestDuration(glyph->restDuration());
            } else {
                path = svgResourceForType(glyph->type());
            }

            if (path) {
                QSvgRenderer renderer(QString::fromUtf8(path));
                if (renderer.isValid()) {
                    painter.save();
                    painter.setClipRect(targetRect);
                    renderer.render(&painter, targetRect);
                    painter.restore();
                }
            }

            if (m_selectedSymbol == glyph) {
                painter.setPen(QPen(QColor(20, 90, 220), 2, Qt::DashLine));
                painter.setBrush(Qt::NoBrush);
                painter.drawRect(targetRect.adjusted(-2, -2, 2, 2));
            }
            continue;
        }

        auto barLine = std::dynamic_pointer_cast<BarLine>(symbol);
        if (barLine) {
            QPen pen(Qt::black, 2);
            if (m_selectedSymbol == barLine) {
                pen.setColor(QColor(20, 90, 220));
            }
            painter.setPen(pen);

            const int x = slotToX(barLine->xSlot());
            const int staffIndex = barLine->staffIndex();
            const int yTop = staffTopY(staffIndex);
            const int yBottom = staffBottomY(staffIndex);
            painter.drawLine(x, yTop, x, yBottom);
            continue;
        }

        auto dot = std::dynamic_pointer_cast<Dot>(symbol);
        if (dot) {
            QPen pen(Qt::black, 2);
            if (m_selectedSymbol == dot) {
                pen.setColor(QColor(20, 90, 220));
            }

            const int x = slotToX(dot->xSlot());
            const int staffIndex = dot->staffIndex();
            const int y = staffStepToY(staffIndex, dot->staffStep());

            const QRectF targetRect(
                x + 8,
                y - 3,
                6,
                6);
            
            painter.setBrush(QBrush(Qt::black, Qt::SolidPattern));
            painter.setPen(pen); 
            painter.drawEllipse(targetRect);
            continue;
        }

        auto timeSig = std::dynamic_pointer_cast<TimeSignature>(symbol);
        if (timeSig) {
            const int x = slotToX(timeSig->xSlot());
            const int staffIndex = timeSig->staffIndex();
            const int yTop = staffTopY(staffIndex);
            const int yBottom = staffBottomY(staffIndex);
            const int yCenter = yTop + 2 * kLineSpacing;

            const QRectF bbox(
                x - kTimeSigBoxWidthPx / 2.0,
                yTop,
                kTimeSigBoxWidthPx,
                yBottom - yTop
            );

            auto fitFontSize = [&](const QString& text, const QRectF& targetRect) -> int {
                // Pick the largest font size that fits into targetRect.
                const qreal marginX = targetRect.width() * 0;
                const qreal marginY = targetRect.height() * 0;
                const QRectF inner = targetRect.adjusted(marginX, marginY, -marginX, -marginY);

                int best = kTimeSigFontSizeMinPt;
                for (int pt = kTimeSigFontSizeMinPt; pt <= kTimeSigFontSizeMaxPt; ++pt) {
                    QFont font = painter.font();
                    font.setPointSize(pt);
                    font.setBold(true);
                    QFontMetricsF fm(font);
                    const QRectF br = fm.boundingRect(text);
                    if (br.width() <= inner.width() + 5 && br.height() <= inner.height() + 5) {
                        best = pt;
                    } else {
                        break;
                    }
                }
                return best;
            };

            painter.save();
            QPen pen(Qt::black, 4);
            painter.setPen(pen);

            painter.drawLine(
                QPointF(x - kTimeSigBarLenPx / 2.0, yCenter),
                QPointF(x + kTimeSigBarLenPx / 2.0, yCenter)
            );

            const QRectF numRect(bbox.left(), yTop, bbox.width(), yCenter - yTop);
            const QRectF denRect(bbox.left(), yCenter, bbox.width(), yBottom - yCenter);

            const int numPt = fitFontSize(QString::number(timeSig->numerator()), numRect);
            const int denPt = fitFontSize(QString::number(timeSig->denominator()), denRect);

            QFont numFont = painter.font();
            numFont.setPointSize(numPt);
            numFont.setBold(true);
            painter.setFont(numFont);
            painter.drawText(
                numRect,
                Qt::AlignCenter | Qt::AlignBottom,
                QString::number(timeSig->numerator())
            );
            QFont denFont = painter.font();
            denFont.setPointSize(denPt);
            denFont.setBold(true);
            painter.setFont(denFont);
            painter.drawText(
                denRect,
                Qt::AlignCenter | Qt::AlignTop,
                QString::number(timeSig->denominator())
            );

            if (m_selectedSymbol == timeSig) {
                painter.setPen(QPen(QColor(20, 90, 220), 2, Qt::DashLine));
                painter.setBrush(Qt::NoBrush);
                painter.drawRect(bbox.adjusted(-2, -2, 2, 2));
            }

            painter.restore();
            continue;
        }
    }
}

void StaffWidget::drawLedgerLinesForStep(QPainter& painter, int x, int staffIndex, int step) {
    painter.setPen(QPen(Qt::black, 2));
    const int halfLedger = 13;

    if (step > kStaffTopStep) {
        for (int ledgerStep = kStaffTopStep + 2; ledgerStep <= step; ledgerStep += 2) {
            const int y = staffStepToY(staffIndex, ledgerStep);
            painter.drawLine(x - halfLedger, y, x + halfLedger, y);
        }
    } else if (step < 0) {
        for (int ledgerStep = -2; ledgerStep >= step; ledgerStep -= 2) {
            const int y = staffStepToY(staffIndex, ledgerStep);
            painter.drawLine(x - halfLedger, y, x + halfLedger, y);
        }
    }
}

namespace {
int durationToInt(Note::Duration d) {
    return static_cast<int>(d);
}

Note::Duration durationFromInt(int v) {
    switch (v) {
    case 0:
        return Note::Duration::Whole;
    case 1:
        return Note::Duration::Half;
    case 2:
        return Note::Duration::Quarter;
    case 3:
        return Note::Duration::Eighth;
    case 4:
        return Note::Duration::Sixteenth;
    default:
        return Note::Duration::Whole;
    }
}
}

bool StaffWidget::exportToPng(const QString& filePath) {
    const QSize renderSize = minimumSizeHint();
    if (renderSize.isEmpty()) {
        return false;
    }

    QImage image(renderSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(252, 252, 252));

    QPainter painter(&image);
    render(painter, renderSize.width(), renderSize.height());

    return image.save(filePath, "PNG");
}

bool StaffWidget::exportToPdf(const QString& filePath) {
    const QSize renderSize = minimumSizeHint();
    if (renderSize.isEmpty()) {
        return false;
    }

    QPdfWriter pdfWriter(filePath);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setResolution(300);

    QPainter painter(&pdfWriter);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const qreal devW = static_cast<qreal>(pdfWriter.width());
    const qreal devH = static_cast<qreal>(pdfWriter.height());
    const qreal scaleX = devW / static_cast<qreal>(renderSize.width());
    const qreal scaleY = devH / static_cast<qreal>(renderSize.height());
    const qreal scale = std::min(scaleX, scaleY);

    const qreal offsetX = (devW - renderSize.width() * scale) / 2.0;
    const qreal offsetY = (devH - renderSize.height() * scale) / 2.0;

    painter.translate(offsetX, offsetY);
    painter.scale(scale, scale);

    render(painter, renderSize.width(), renderSize.height());
    painter.end();

    return true;
}

QByteArray StaffWidget::toJson() const {
    QJsonObject root;
    root["staffCount"] = m_staffCount;

    QJsonArray symbols;
    for (const auto& symbol : m_score.symbols()) {
        if (auto note = std::dynamic_pointer_cast<Note>(symbol)) {
            QJsonObject obj;
            obj["kind"] = "note";
            obj["xSlot"] = note->xSlot();
            obj["staffIndex"] = note->staffIndex();
            obj["staffStep"] = note->staffStep();
            obj["duration"] = durationToInt(note->duration());
            symbols.push_back(obj);
            continue;
        }

        if (auto glyph = std::dynamic_pointer_cast<GlyphSymbol>(symbol)) {
            QJsonObject obj;
            obj["kind"] = "glyph";
            obj["symbolType"] = static_cast<int>(glyph->type());
            obj["xSlot"] = glyph->xSlot();
            obj["staffIndex"] = glyph->staffIndex();
            obj["staffStep"] = glyph->staffStep();
            if (glyph->type() == MusicSymbol::SymbolType::Rest) {
                obj["restDuration"] = durationToInt(glyph->restDuration());
            }
            symbols.push_back(obj);
            continue;
        }

        if (auto barLine = std::dynamic_pointer_cast<BarLine>(symbol)) {
            QJsonObject obj;
            obj["kind"] = "barline";
            obj["xSlot"] = barLine->xSlot();
            obj["staffIndex"] = barLine->staffIndex();
            symbols.push_back(obj);
            continue;
        }

        if (auto dot = std::dynamic_pointer_cast<Dot>(symbol)) {
            QJsonObject obj;
            obj["kind"] = "dot";
            obj["xSlot"] = dot->xSlot();
            obj["staffIndex"] = dot->staffIndex();
            obj["staffStep"] = dot->staffStep();
            symbols.push_back(obj);
            continue;
        }

        if (auto ts = std::dynamic_pointer_cast<TimeSignature>(symbol)) {
            QJsonObject obj;
            obj["kind"] = "timesig";
            obj["xSlot"] = ts->xSlot();
            obj["staffIndex"] = ts->staffIndex();
            obj["numerator"] = ts->numerator();
            obj["denominator"] = ts->denominator();
            symbols.push_back(obj);
            continue;
        }
    }

    root["symbols"] = symbols;
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

bool StaffWidget::fromJson(const QByteArray& jsonData) {
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError || doc.isNull() || !doc.isObject()) {
        return false;
    }

    const QJsonObject root = doc.object();
    m_staffCount = root.value("staffCount").toInt(1);
    m_score = Score(); // clear existing symbols
    m_selectedSymbol.reset();
    m_selectedNote.reset();
    m_dragging = false;

    const QJsonArray symbols = root.value("symbols").toArray();
    for (const QJsonValue& v : symbols) {
        if (!v.isObject()) {
            continue;
        }
        const QJsonObject obj = v.toObject();
        const QString kind = obj.value("kind").toString();

        if (kind == "note") {
            const int xSlot = obj.value("xSlot").toInt();
            const int staffIndex = obj.value("staffIndex").toInt();
            const int staffStep = obj.value("staffStep").toInt();
            const int d = obj.value("duration").toInt();
            m_score.addNote(xSlot, staffIndex, staffStep, durationFromInt(d));
        } else if (kind == "glyph") {
            const int symbolTypeInt = obj.value("symbolType").toInt();
            const auto symbolType = static_cast<MusicSymbol::SymbolType>(symbolTypeInt);
            const int xSlot = obj.value("xSlot").toInt();
            const int staffIndex = obj.value("staffIndex").toInt();
            const int staffStep = obj.value("staffStep").toInt();

            if (symbolType == MusicSymbol::SymbolType::Rest) {
                const int restDuration = obj.value("restDuration").toInt();
                m_score.addGlyph(symbolType, xSlot, staffIndex, staffStep, durationFromInt(restDuration));
            } else {
                m_score.addGlyph(symbolType, xSlot, staffIndex, staffStep);
            }
        } else if (kind == "barline") {
            const int xSlot = obj.value("xSlot").toInt();
            const int staffIndex = obj.value("staffIndex").toInt();
            m_score.addBarLine(xSlot, staffIndex);
        } else if (kind == "dot") {
            const int xSlot = obj.value("xSlot").toInt();
            const int staffIndex = obj.value("staffIndex").toInt();
            const int staffStep = obj.value("staffStep").toInt();
            m_score.addDot(xSlot, staffIndex, staffStep);
        } else if (kind == "timesig") {
            const int xSlot = obj.value("xSlot").toInt();
            const int staffIndex = obj.value("staffIndex").toInt();
            const int numerator = obj.value("numerator").toInt();
            const int denominator = obj.value("denominator").toInt();
            m_score.addTimeSignature(xSlot, staffIndex, numerator, denominator);
        }
    }

    update();
    return true;
}
