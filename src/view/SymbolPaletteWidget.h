#pragma once

#include "../common/ToolType.h"

#include <QWidget>

class QButtonGroup;

class SymbolPaletteWidget : public QWidget {
    Q_OBJECT

public:
    explicit SymbolPaletteWidget(QWidget* parent = nullptr);

signals:
    void toolChanged(ToolType tool);
};
