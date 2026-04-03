#include "SymbolPaletteWidget.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSvgRenderer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QIcon>
#include <QPixmap>
#include <QPainter>

static QIcon svgIcon(const QString& path, int size = 32) {
    QSvgRenderer renderer(path);
    if (!renderer.isValid()) return {};
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    renderer.render(&p);
    return QIcon(pix);
}

SymbolPaletteWidget::SymbolPaletteWidget(QWidget* parent)
    : QWidget(parent) {

    // Main horizontal layout: two groups side by side
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(6, 4, 6, 4);
    mainLayout->setSpacing(12);

    auto* group = new QButtonGroup(this);
    group->setExclusive(true);

    struct ToolButtonDef {
        const char* label;      // short label shown under icon
        const char* tooltip;    // full name shown on hover
        const char* svgPath;    // nullptr = no icon
        ToolType tool;
    };

    // ---- Notes ----
    const ToolButtonDef notes[] = {
        {"Select",  "Select / Move",        nullptr,                              ToolType::Select},
        {"Whole",   "Whole note",            ":/assets/svg/whole_note.svg",        ToolType::InsertWholeNote},
        {"Half",    "Half note",             ":/assets/svg/half_note.svg",         ToolType::InsertHalfNote},
        {"Quarter", "Quarter note",          ":/assets/svg/quarter_note.svg",      ToolType::InsertQuarterNote},
        {"8th",     "Eighth note",           ":/assets/svg/eighth_note.svg",       ToolType::InsertEighthNote},
        {"16th",    "Sixteenth note",        ":/assets/svg/sixteenth_note.svg",    ToolType::InsertSixteenthNote},
        {"?",       "Undefined note",        ":/assets/svg/undefined_note.svg",    ToolType::InsertUndefinedNote},
    };

    // ---- Rests ----
    const ToolButtonDef rests[] = {
        {"W.rest",  "Whole rest",            ":/assets/svg/whole_rest.svg",        ToolType::InsertWholeRest},
        {"H.rest",  "Half rest",             ":/assets/svg/half_rest.svg",         ToolType::InsertHalfRest},
        {"Q.rest",  "Quarter rest",          ":/assets/svg/quarter_rest.svg",      ToolType::InsertQuarterRest},
        {"8.rest",  "Eighth rest",           ":/assets/svg/eighth_rest.svg",       ToolType::InsertEighthRest},
        {"16.rest", "Sixteenth rest",        ":/assets/svg/sixteenth_rest.svg",    ToolType::InsertSixteenthRest},
    };

    // ---- Accidentals & other ----
    const ToolButtonDef others[] = {
        {"diez",       "Sharp",                 ":/assets/svg/sharp.svg",             ToolType::InsertSharp},
        {"bemol",       "Flat",                  ":/assets/svg/flat.svg",              ToolType::InsertFlat},
        {"becare",       "Natural (Beccare)",     ":/assets/svg/beccare.svg",           ToolType::InsertBeccare},
        {"barline",       "Bar line",              ":/assets/svg/barline.svg",                    ToolType::InsertBarLine},
        {"dot",       "Dot for note",            ":/assets/svg/quarter_note_with_dot.svg", ToolType::InsertDot},
        {"4/4",     "Time signature",        nullptr,                              ToolType::InsertTimeSignature},
        {"eraser",       "Delete symbol",        ":/assets/svg/eraser.svg",          ToolType::Eraser},
    };

    // Helper: build a labeled group box with buttons
    auto makeGroup = [&](const char* title,
                         const ToolButtonDef* defs, int count,
                         QWidget* parent) -> QWidget* {
        auto* box = new QWidget(parent);
        auto* vl  = new QVBoxLayout(box);
        vl->setContentsMargins(0, 0, 0, 0);
        vl->setSpacing(2);

        auto* titleLabel = new QLabel(QString::fromUtf8(title), box);
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet("font-size:10px; color:#999; font-weight:bold;");
        vl->addWidget(titleLabel);

        auto* row = new QHBoxLayout();
        row->setSpacing(3);
        row->setContentsMargins(0, 0, 0, 0);

        for (int i = 0; i < count; ++i) {
            const ToolButtonDef& d = defs[i];
            auto* btn = new QToolButton(box);
            btn->setCheckable(true);
            btn->setToolTip(QString::fromUtf8(d.tooltip));
            btn->setFixedSize(52, 52);
            btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            btn->setText(QString::fromUtf8(d.label));

            if (d.svgPath) {
                btn->setIcon(svgIcon(QString::fromUtf8(d.svgPath), 28));
                btn->setIconSize(QSize(28, 28));
            } else {
                // Large text as "icon"
                btn->setFont(QFont("serif", 32));
            }

            btn->setStyleSheet(
                "QToolButton {"
                "  border: 1px solid #dcdfe6;"
                "  border-radius: 8px;"
                "  background: #ffffff;"
                "  font-size: 10px;"
                "  color: #303133;"
                "}"
                "QToolButton:hover {"
                "  background: #ecf5ff;"
                "  border-color: #409eff;"
                "  color: #409eff;"
                "}"
                "QToolButton:checked {"
                "  background: #ecf5ff;"
                "  border: 2px solid #409eff;"
                "  font-weight: bold;"
                "  color: #409eff;"
                "}"
            );

            row->addWidget(btn);
            group->addButton(btn, static_cast<int>(d.tool));
        }
        row->addStretch(1);
        vl->addLayout(row);
        return box;
    };

    mainLayout->addWidget(makeGroup("Notes", notes, 7, this));
    mainLayout->addWidget(makeGroup("Rests", rests, 5, this));
    mainLayout->addWidget(makeGroup("Accidentals & other", others, 7, this));
    mainLayout->addStretch(1);

    connect(group, &QButtonGroup::idClicked, this, [this](int id) {
        emit toolChanged(static_cast<ToolType>(id));
    });

    if (auto* selectButton = group->button(static_cast<int>(ToolType::Select))) {
        selectButton->setChecked(true);
    }

    setStyleSheet("SymbolPaletteWidget { background: #fafafa; border-bottom: 1px solid #ccc; }");
}
