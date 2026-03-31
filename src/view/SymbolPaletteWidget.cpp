#include "SymbolPaletteWidget.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPushButton>

SymbolPaletteWidget::SymbolPaletteWidget(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    auto* group = new QButtonGroup(this);
    group->setExclusive(true);

    struct ToolButtonDef {
        const char* text;
        ToolType tool;
    };

    const ToolButtonDef defs[] = {
        {"Select", ToolType::Select},
        {"Whole note", ToolType::InsertWholeNote},
        {"Half note", ToolType::InsertHalfNote},
        {"Quarter note", ToolType::InsertQuarterNote},
        {"Eighth note", ToolType::InsertEighthNote},
        {"Sixteenth note", ToolType::InsertSixteenthNote},
        {"Undefined note", ToolType::InsertUndefinedNote},
        {"Whole rest", ToolType::InsertWholeRest},
        {"Half rest", ToolType::InsertHalfRest},
        {"Quarter rest", ToolType::InsertQuarterRest},
        {"Eighth rest", ToolType::InsertEighthRest},
        {"Sixteenth rest", ToolType::InsertSixteenthRest},
        {"Sharp", ToolType::InsertSharp},
        {"Flat", ToolType::InsertFlat},
        {"Beccare", ToolType::InsertBeccare},
        {"Bar line", ToolType::InsertBarLine},
        {"Dot", ToolType::InsertDot},
        {"Time signature", ToolType::InsertTimeSignature},
        {"Eraser", ToolType::Eraser},
    };

    for (const ToolButtonDef& def : defs) {
        auto* button = new QPushButton(QString::fromUtf8(def.text), this);
        button->setCheckable(true);
        layout->addWidget(button);
        group->addButton(button, static_cast<int>(def.tool));
    }

    connect(group, &QButtonGroup::idClicked, this, [this](int id) {
        emit toolChanged(static_cast<ToolType>(id));
    });

    if (auto* selectButton = group->button(static_cast<int>(ToolType::Select))) {
        selectButton->setChecked(true);
    }
}
