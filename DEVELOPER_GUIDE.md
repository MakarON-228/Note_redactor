# Note Redactor — Developer Guide

> Полное руководство для разработчиков: архитектура, модули, ключевые алгоритмы, константы, точки расширения.

---

## Содержание

1. [Обзор проекта](#1-обзор-проекта)
2. [Технологический стек](#2-технологический-стек)
3. [Структура проекта](#3-структура-проекта)
4. [Сборка и запуск](#4-сборка-и-запуск)
5. [Архитектура приложения](#5-архитектура-приложения)
6. [Модель данных (model/)](#6-модель-данных-model)
7. [Представление (view/)](#7-представление-view)
8. [Главное окно (MainWindow)](#8-главное-окно-mainwindow)
9. [Аудио-подсистема (audio/)](#9-аудио-подсистема-audio)
10. [Система координат и нотного стана](#10-система-координат-и-нотного-стана)
11. [Кэширование SVG](#11-кэширование-svg)
12. [Сериализация JSON](#12-сериализация-json)
13. [Экспорт PNG/PDF](#13-экспорт-pngpdf)
14. [Точки расширения](#14-точки-расширения)
15. [Известные особенности и подводные камни](#15-известные-особенности-и-подводные-камни)

---

## 1. Обзор проекта

**Note Redactor** — десктопный редактор нотного стана, написанный на C++17 с использованием Qt6 Widgets. Приложение позволяет:

- Визуально размещать ноты, паузы, знаки альтерации, тактовые черты, размеры и точки на нотном стане
- Управлять несколькими нотными станами (staff rows)
- Записывать звук с микрофона и автоматически определять высоту тона (алгоритм YIN), добавляя ноты на стан
- Экспортировать результат в PNG, PDF или сохранять/загружать проект в JSON

---

## 2. Технологический стек

| Компонент | Версия / Технология |
|---|---|
| Язык | C++17 |
| Фреймворк | Qt6 (Widgets, Svg, Multimedia) |
| Система сборки | CMake ≥ 3.16 |
| Алгоритм определения высоты | YIN (реализован вручную) |
| Графика | QPainter + QSvgRenderer |
| Ресурсы | Qt Resource System (`.qrc`) |

---

## 3. Структура проекта

```
NoteRedactor/
├── CMakeLists.txt              # Главный файл сборки
├── README.md                   # Краткое описание
├── resources/
│   └── symbols.qrc             # Qt-ресурсы (SVG-файлы)
├── assets/
│   └── svg/                    # Исходные SVG-файлы (18 файлов)
├── src/
│   ├── main.cpp                # Точка входа
│   ├── MainWindow.h / .cpp     # Главное окно
│   ├── common/
│   │   └── ToolType.h          # Перечисление инструментов
│   ├── model/
│   │   ├── MusicSymbol.h       # Базовый класс символа
│   │   ├── Note.h / .cpp       # Нота
│   │   ├── GlyphSymbol.h       # SVG-символ (ключ, пауза, альтерация)
│   │   ├── BarLine.h           # Тактовая черта
│   │   ├── TimeSignature.h     # Размер
│   │   ├── Dot.h               # Точка
│   │   └── Score.h / .cpp      # Контейнер всех символов
│   ├── view/
│   │   ├── StaffWidget.h / .cpp    # Виджет нотного стана (основная логика отрисовки)
│   │   └── SymbolPaletteWidget.h / .cpp  # Панель инструментов
│   └── audio/
│       ├── AudioRecorder.h / .cpp    # Запись с микрофона
│       └── PitchDetector.h / .cpp    # YIN-детектор высоты
└── build/                      # Директория сборки (игнорируется)
```

---

## 4. Сборка и запуск

```bash
# Конфигурация
cmake -S . -B build

# Сборка
cmake --build build -j

# Запуск
./build/NoteRedactor
```

**Зависимости:** Qt6 с компонентами `Widgets`, `Svg`, `Multimedia`. Убедитесь, что они установлены:

```bash
# Ubuntu/Debian
sudo apt install qt6-base-dev qt6-svg-dev qt6-multimedia-dev

# Arch Linux
sudo pacman -S qt6-base qt6-svg qt6-multimedia
```

---

## 5. Архитектура приложения

```
┌─────────────────────────────────────────────────────────┐
│                     MainWindow                          │
│  ┌─────────────────┐  ┌──────────────┐  ┌────────────┐ │
│  │SymbolPaletteWdg │  │  ScrollArea  │  │ ControlBar │ │
│  │  (панель        │  │  ┌────────┐  │  │ (кнопки,   │ │
│  │   инструментов) │  │  │Staff   │  │  │  запись,   │ │
│  │                 │  │  │Widget  │  │  │  экспорт)  │ │
│  │  toolChanged ───┼──┤setTool()   │  │              │ │
│  └─────────────────┘  │  └────────┘  │  └────────────┘ │
│                       └──────────────┘                  │
│                                                         │
│  ┌───────────────────────────────────────────────────┐  │
│  │              AudioRecorder                        │  │
│  │  ┌─────────────┐    ┌──────────────┐              │  │
│  │  │ QAudioSource│───▶│PitchDetector │──noteDetected│  │
│  │  └─────────────┘    │  (YIN)       │              │  │
│  │                     └──────────────┘              │  │
│  └───────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

### Поток данных

1. **UI → Модель**: Пользователь кликает на палитре → `SymbolPaletteWidget` эмитит `toolChanged(ToolType)` → `StaffWidget::setTool()` → при клике на стан создаётся символ через `Score::add*()`
2. **Модель → UI**: `StaffWidget::paintEvent()` вызывает `render()` → перебирает `Score::symbols()` → рисует каждый символ
3. **Аудио → Модель**: Микрофон → `AudioRecorder` → `PitchDetector` (YIN) → сигнал `noteDetected(midiNote)` → `StaffWidget::addNoteFromMidi()`

---

## 6. Модель данных (model/)

### 6.1. Иерархия классов

```
MusicSymbol (базовый, абстрактный)
├── Note              — нота (позиция + длительность + высота)
├── GlyphSymbol       — SVG-символ (ключ, пауза, диез, бемоль, бекар)
├── BarLine           — тактовая черта
├── TimeSignature     — размер (числитель/знаменатель)
└── Dot               — точка увеличения длительности
```

### 6.2. MusicSymbol (`MusicSymbol.h`)

Базовый класс для всех музыкальных символов.

```cpp
enum class SymbolType {
    Note, TrebleClef, Rest, Sharp, Flat, Beccare,
    BarLine, TimeSignature, Dot
};
```

Хранит только `SymbolType m_type`. Виртуальный деструктор позволяет корректное удаление через базовый указатель.

### 6.3. Note (`Note.h`, `Note.cpp`)

**Ключевые поля:**
| Поле | Тип | Описание |
|---|---|---|
| `m_xSlot` | `int` | Позиция по горизонтали (дискретная сетка) |
| `m_staffIndex` | `int` | Индекс нотного стана (0-based) |
| `m_staffStep` | `int` | Вертикальная позиция (полушаги: 0 = C4, 2 = E4 и т.д.) |
| `m_pitchClass` | `PitchClass` | Диатоническая ступень (C, D, E, F, G, A, B) |
| `m_octave` | `int` | Октава |
| `m_accidental` | `Accidental` | Флаг (-1 = ♭, 0 = ♮, 1 = ♯) |
| `m_duration` | `Duration` | Длительность (Whole, Half, Quarter, Eighth, Sixteenth, Undefined) |
| `m_frequencyHz` | `double` | Частота в Гц |

**Перечисления:**
- `Duration`: `Whole` (0), `Half` (1), `Quarter` (2), `Eighth` (3), `Sixteenth` (4), `Undefined` (5)
- `PitchClass`: C=0, D=1, E=2, F=3, G=4, A=5, B=6
- `Accidental`: Flat=-1, Natural=0, Sharp=1

**Важные методы:**
- `updatePitchFromStaffStep()` — пересчитывает `pitchClass`, `octave`, `accidental` из `staffStep`
- `calculateFrequency()` — вычисляет частоту из MIDI-ноты
- `noteName()` — возвращает строку вида "C4", "F#5" и т.д.
- `isOnLine()` / `isBetweenLines()` — определяет, находится ли нота на линии или между линиями

### 6.4. GlyphSymbol (`GlyphSymbol.h`)

Универсальный класс для SVG-символов. Хранит:
- `m_xSlot`, `m_staffIndex`, `m_staffStep` — позиция
- `m_restDuration` — длительность (используется только для пауз)

`SymbolType` определяет, какой SVG-файл рендерить:
| SymbolType | SVG-файл |
|---|---|
| `TrebleClef` | `treble_clef.svg` |
| `Rest` | Зависит от `restDuration` (см. `svgResourceForRestDuration`) |
| `Sharp` | `sharp.svg` |
| `Flat` | `flat.svg` |
| `Beccare` | `beccare.svg` |

### 6.5. BarLine (`BarLine.h`)

Простой класс: `m_xSlot` + `m_staffIndex`. Вертикальная линия от верхней до нижней линии стана.

### 6.6. TimeSignature (`TimeSignature.h`)

Хранит `m_numerator` и `m_denominator` (числитель и знаменатель). Отрисовывается как два числа друг над другом с горизонтальной линией между ними.

### 6.7. Dot (`Dot.h`)

Точка увеличения длительности. Позиция: `m_xSlot`, `m_staffIndex`, `m_staffStep`. Отрисовывается как чёрный эллипс справа от ноты.

### 6.8. Score (`Score.h`, `Score.cpp`)

**Единый контейнер** всех музыкальных символов.

```cpp
QVector<std::shared_ptr<MusicSymbol>> m_symbols;
```

**API добавления:**
- `addNote(xSlot, staffIndex, staffStep, duration?)` → `shared_ptr<Note>`
- `addBarLine(xSlot, staffIndex)` → `shared_ptr<BarLine>`
- `addDot(xSlot, staffIndex, staffStep)` → `shared_ptr<Dot>`
- `addGlyph(type, xSlot, staffIndex, staffStep, restDuration?)` → `shared_ptr<GlyphSymbol>`
- `addTimeSignature(xSlot, staffIndex, numerator, denominator)` → `shared_ptr<TimeSignature>`

Каждый метод имеет перегрузку без `staffIndex` (по умолчанию 0).

**API поиска (hit-testing):**
- `noteAt(xSlot, staffIndex, staffStep, slotTolerance, stepTolerance)`
- `glyphAt(...)`, `barLineAt(...)`, `dotAt(...)`, `timeSignatureAt(...)`

Все методы поиска принимают `slotTolerance` и `stepTolerance` для放宽 поиска.

**API удаления:**
- `removeNoteAt(...)`, `removeBarLineAt(...)`, `removeDotAt(...)`, `removeTimeSignatureAt(...)` — по позиции
- `removeSymbol(shared_ptr<MusicSymbol>)` — по указателю

---

## 7. Представление (view/)

### 7.1. StaffWidget (`StaffWidget.h`, `StaffWidget.cpp`)

**Самый большой и сложный файл проекта** (~1200 строк). Отвечает за:
- Отрисовку нотного стана и всех символов
- Обработку мыши (клик, перетаскивание, выделение прямоугольником)
- Обработку клавиатуры (стрелки, цифры, Delete)
- Экспорт в PNG/PDF
- Сериализацию в JSON

#### 7.1.1. Ключевые константы (анонимный namespace)

| Константа | Значение | Описание |
|---|---|---|
| `kStaffFirstTopY` | 90 | Y-координата верхней линии первого стана |
| `kStaffLeftMargin` | 70 | Отступ слева от края виджета |
| `kLineSpacing` | 16 | Расстояние между линиями стана (в пикселях) |
| `kHalfStepPx` | 8 | Половина расстояния между линиями (`kLineSpacing / 2`) |
| `kStaffLinesCount` | 5 | Количество линий в стане |
| `kStaffRowPitchBase` | 175 | Базовое расстояние между верхними линиями соседних станов |
| `kSlotWidth` | 28 | Ширина одной горизонтальной ячейки (slot) |
| `kNoteWidth` | 24 | Ширина овала ноты |
| `kNoteHeight` | 12 | Высота овала ноты |
| `kStaffTopStep` | 8 | Верхний шаг стана (`(5-1)*2`) |
| `kDefaultTrebleClefSlot` | 0 | Слот скрипичного ключа |
| `kDefaultTrebleClefStaffStep` | 3 | Шаг скрипичного ключа (линия G4) |
| `kTimeSigBoxWidthPx` | 40 | Ширина блока размера |
| `kRightStaffBorder` | 50 | Правый отступ |
| `kLedgerLinesBorderBase` | 6 | Базовое количество добавочных линий |

**Функции масштабирования:**
```cpp
staffRowPitch(k) = 175 + 17 * k     // расстояние между станами при spacing=k
ledgerLinesBorder(k) = 6 + k        // допустимое количество добавочных линий
```

#### 7.1.2. Система координат

**Горизонталь (X):**
- Дискретная сетка «слотов»: `slotToX(slot) = 70 + slot * 28`
- Обратное преобразование: `xToSlot(x) = round((x - 70) / 28)`

**Вертикаль (Y):**
- `staffTopY(staffIndex)` — Y верхней линии стана
- `staffBottomY(staffIndex)` — Y нижней линии стана
- `staffStepToY(staffIndex, step)` — Y для конкретного шага
- `yToStaffIndex(y)` — определяет индекс стана по Y
- `yToStaffStep(staffIndex, y)` — определяет шаг по Y

**Система шагов (staffStep):**
- Шаг 0 = C4 (первая добавочная линия снизу)
- Шаг 2 = E4 (нижняя линия стана)
- Шаг 3 = F4 (первый промежуток)
- Шаг 4 = G4 (вторая линия — здесь стоит скрипичный ключ)
- Шаг 8 = C5 (верхняя линия стана)
- Чётные шаги = на линии, нечётные = между линиями

#### 7.1.3. Отрисовка

**`render(painter, width, height)`** — главный метод рендеринга:
1. `drawStaff()` — рисует 5 горизонтальных линий для каждого стана
2. `drawSymbols()` — перебирает все символы из `Score` и рисует каждый
3. Рисует прямоугольник выделения (если активен)
4. Рисует рамки вокруг мульти-выбранных символов

**`drawSymbols()`** — для каждого символа:
- **Note**: рисует SVG-овал через `getCachedSvgPixmap()`. Для нот выше G4 (step > 4) — флип по вертикали (stem direction). Добавочные линии рисуются отдельно через `drawLedgerLinesForStep()`.
- **GlyphSymbol**: выбирает SVG по типу, рисует через кэш
- **BarLine**: вертикальная линия `QPainter::drawLine()`
- **Dot**: чёрный эллипс `QPainter::drawEllipse()`
- **TimeSignature**: два числа с автоподбором размера шрифта (`fitFontSize`), горизонтальная линия между ними

#### 7.1.4. SVG-кэш

```cpp
std::map<QString, QPixmap> m_svgCache;
```

Ключ: `"path:widthxheight"`. При каждом `setSpacingCoefficient()` кэш очищается, т.к. размеры могут измениться.

#### 7.1.5. Обработка мыши

**`mousePressEvent`:**
- Определяет слот, стан, шаг по позиции клика
- В зависимости от `m_currentTool`:
  - Инструменты вставки → создают символ через `Score::add*()`
  - `Select` → hit-testing (поиск символа под курсором) или начало rectangle selection
  - `Eraser` → удаление символа под курсором

**`mouseMoveEvent`:**
- Обновляет курсорную позицию (`m_cursorSlot`, `m_cursorStaffIndex`, `m_cursorStep`)
- Если `m_dragging` — перемещает выбранный символ
- Если `m_selecting` — обновляет `m_selectionRect`

**`mouseReleaseEvent`:**
- Завершает rectangle selection, находит все символы внутри `m_selectionRect`, добавляет в `m_selectedSymbols`

#### 7.1.6. Обработка клавиатуры

| Клавиша | Действие |
|---|---|
| `↑` / `↓` | Перемещение выбранного символа вверх/вниз |
| `←` / `→` | Перемещение влево/вправо (с переходом между станами) |
| `0-9` | Быстрая вставка через `addQuickSymbol()` |
| `Delete` / `Backspace` | Удаление выбранного символа |

**`addQuickSymbol(int digit)`:**
| Цифра | Символ |
|---|---|
| 1 | Whole note |
| 2 | Half note |
| 3 | Quarter note |
| 4 | Eighth note |
| 5 | Sixteenth note |
| 6 | Whole rest |
| 7 | Half rest |
| 8 | Quarter rest |
| 9 | Eighth rest |
| 0 | Sixteenth rest |

#### 7.1.7. Состояния выделения

```cpp
std::shared_ptr<MusicSymbol> m_selectedSymbol;       // Одиночное выделение
std::shared_ptr<Note> m_selectedNote;                // Указатель на ноту (если выбрана нота)
QVector<std::shared_ptr<MusicSymbol>> m_selectedSymbols; // Мульти-выделение
QRect m_selectionRect;                               // Прямоугольник выделения
bool m_selecting;                                    // Флаг процесса выделения
bool m_dragging;                                     // Флаг перетаскивания
```

### 7.2. SymbolPaletteWidget (`SymbolPaletteWidget.h`, `.cpp`)

Горизонтальная панель инструментов в верхней части окна.

**Три группы кнопок:**
1. **Notes**: Select, Whole, Half, Quarter, 8th, 16th, Undefined
2. **Rests**: W.rest, H.rest, Q.rest, 8.rest, 16.rest
3. **Accidentals & other**: diez, bemol, becare, barline, dot, 4/4, eraser

Каждая кнопка — `QToolButton` с иконкой из SVG и текстовой подписью. Группа `QButtonGroup` обеспечивает эксклюзивный выбор (radio-режим).

**Сигнал:**
```cpp
void toolChanged(ToolType tool);
```

---

## 8. Главное окно (MainWindow)

### 8.1. Компоновка

```
┌──────────────────────────────────────────────────────┐
│  SymbolPaletteWidget  (панель инструментов)          │
├──────────────────────────────────────────────────────┤
│                                                      │
│  QScrollArea → StaffWidget  (нотный стан)            │
│                                                      │
├──────────────────────────────────────────────────────┤
│  ControlBar (высота 56px)                            │
│  [↑] [↓] [✕] [+Staff]  [🖼PNG] [📄PDF] [💾Save]     │
│  [📂Load]  Staff spacing: [-] 0 [+]  [● Start Rec]  │
└──────────────────────────────────────────────────────┘
```

Фиксированная ширина окна: **1400px**.

### 8.2. ControlBar

Кнопки управления (снизу):
| Кнопка | Действие |
|---|---|
| `↑` / `↓` | `StaffWidget::moveSelectedNoteUp/Down()` |
| `✕` | `StaffWidget::deleteSelectedNote()` |
| `+ Staff` | `StaffWidget::addStaffRow()` |
| `Export PNG` | `StaffWidget::exportToPng()` через `QFileDialog` |
| `Export PDF` | `StaffWidget::exportToPdf()` через `QFileDialog` |
| `Save` | `StaffWidget::toJson()` → файл `.json` |
| `Load` | Файл `.json` → `StaffWidget::fromJson()` |
| `Staff spacing: [-] 0 [+]` | `StaffWidget::setSpacingCoefficient(k)` |
| `● Start Recording` | `AudioRecorder::startRecording()` / `stopRecording()` |

---

## 9. Аудио-подсистема (audio/)

### 9.1. AudioRecorder (`AudioRecorder.h`, `.cpp`)

Обёртка над `QAudioSource` для захвата звука с микрофона.

**Параметры:**
- Sample rate: 44100 Гц
- Формат: Float, моно
- Buffer size: 4096 семплов
- Интервал обработки: 50 мс (таймер)

**Алгоритм обработки:**
1. Читает `m_bufferSize` семплов из `QAudioSource`
2. Передаёт в `PitchDetector::detectPitch()`
3. Если частота обнаружена → конвертирует в MIDI-ноту
4. **Дебаунсинг**: требует 3 последовательных detections одной ноты перед эмиссией сигнала

**Сигнал:**
```cpp
void noteDetected(int midiNote);
```

### 9.2. PitchDetector (`PitchDetector.h`, `.cpp`)

Реализация **алгоритма YIN** для определения высоты тона.

**Шаги алгоритма:**
1. **Difference function**: `d[τ] = Σ(x[i] - x[i+τ])²`
2. **Cumulative mean normalized difference**: `d'[τ] = d[τ] * τ / Σ(d[j])`
3. **Absolute threshold**: поиск первого `τ` где `d'[τ] < 0.1`
4. **Parabolic interpolation**: уточнение `τ` для большей точности
5. **Частота**: `f = sampleRate / τ`

**`frequencyToMidiNote(frequency)`:**
```cpp
midiNote = round(69 + 12 * log2(frequency / 440.0))
```

---

## 10. Система координат и нотного стана

### 10.1. Вертикальная система (staffStep)

```
  step 10  ─────  B5  (добавочная линия)
  step  9  ────  A5
  step  8  ─────  G5  (верхняя линия стана)
  step  7  ────  F5
  step  6  ─────  E5
  step  5  ────  D5
  step  4  ─────  C5
  step  3  ────  B4
  step  2  ─────  A4  (нижняя линия стана)
  step  1  ────  G4
  step  0  ─────  F4  (скрипичный ключ на step 3, но G4 = step 4)
  step -1  ────  E4
  step -2  ─────  D4  (добавочная линия)
```

**Важно:** Скрипичный ключ размещается на `step = 3` (линия G4).

### 10.2. Горизонтальная система (xSlot)

```
  x=70    x=98    x=126   x=154   x=182   ...
  slot=0  slot=1  slot=2  slot=3  slot=4  ...
  (clef)  (нота)  (нота)  (нота)  (нота)
```

---

## 11. Кэширование SVG

```cpp
std::map<QString, QPixmap> m_svgCache;
```

**Ключ:** `"path:widthxheight"` (например, `":/assets/svg/quarter_note.svg:24x60"`)

**Методы:**
- `getCachedSvgPixmap(path, size)` — возвращает кэшированный или создаёт новый `QPixmap`
- `clearSvgCache()` — очищает весь кэш (вызывается при изменении `spacingK`)

---

## 12. Сериализация JSON

### 12.1. Формат

```json
{
  "staffCount": 1,
  "spacingK": 0,
  "symbols": [
    { "kind": "note", "xSlot": 5, "staffIndex": 0, "staffStep": 4, "duration": 2 },
    { "kind": "glyph", "symbolType": 1, "xSlot": 0, "staffIndex": 0, "staffStep": 3 },
    { "kind": "glyph", "symbolType": 2, "xSlot": 5, "staffIndex": 0, "staffStep": 4, "restDuration": 0 },
    { "kind": "barline", "xSlot": 10, "staffIndex": 0 },
    { "kind": "dot", "xSlot": 6, "staffIndex": 0, "staffStep": 4 },
    { "kind": "timesig", "xSlot": 3, "staffIndex": 0, "numerator": 4, "denominator": 4 }
  ]
}
```

### 12.2. Маппинг duration

| Int | Duration |
|---|---|
| 0 | Whole |
| 1 | Half |
| 2 | Quarter |
| 3 | Eighth |
| 4 | Sixteenth |

### 12.3. Маппинг SymbolType

| Int | SymbolType |
|---|---|
| 0 | Note |
| 1 | TrebleClef |
| 2 | Rest |
| 3 | Sharp |
| 4 | Flat |
| 5 | Beccare |
| 6 | BarLine |
| 7 | TimeSignature |
| 8 | Dot |

---

## 13. Экспорт PNG/PDF

### 13.1. PNG

```cpp
bool exportToPng(const QString& filePath);
```

Создаёт `QImage` размером `minimumSizeHint()`, рисует через `render()`, сохраняет в файл.

### 13.2. PDF

```cpp
bool exportToPdf(const QString& filePath);
```

Использует `QPdfWriter` с форматом A4, 300 DPI. Масштабирует содержимое с сохранением пропорций (min scale по X/Y), центрирует на странице.

---

## 14. Точки расширения

### 14.1. Добавление нового типа символа

1. Создать заголовочный файл в `model/` (наследник от `MusicSymbol`)
2. Добавить новый вариант в `SymbolType`
3. Добавить метод `add*()` в `Score`
4. Добавить обработку в `StaffWidget::mousePressEvent()` (новый `ToolType`)
5. Добавить отрисовку в `StaffWidget::drawSymbols()`
6. Добавить кнопку в `SymbolPaletteWidget`
7. Добавить сериализацию в `toJson()`/`fromJson()`

### 14.2. Добавление нового инструмента

1. Добавить значение в `enum class ToolType` (`common/ToolType.h`)
2. Добавить кнопку в `SymbolPaletteWidget`
3. Добавить `case` в `StaffWidget::mousePressEvent()`

### 14.3. Изменение внешнего стана

Все константы в анонимном namespace в начале `StaffWidget.cpp`:
- `kLineSpacing` — расстояние между линиями
- `kSlotWidth` — ширина слота
- `kStaffRowPitchBase` — расстояние между станами
- Цвета линий, толщина и т.д. — в `drawStaff()`

### 14.4. Замена SVG-ресурсов

1. Заменить файл в `assets/svg/`
2. Обновить `resources/symbols.qrc` если имя изменилось
3. Пересобрать проект (RCC перекомпилируется автоматически)

---

## 15. Известные особенности и подводные камни

### 15.1. Скрипичный ключ

- Всегда автоматически добавляется в слот 0 каждого стана при создании
- Не может быть удалён или перемещён (проверки в `moveSelectedNote*()`, `deleteSelectedNote()`, `keyPressEvent()`)
- При `addStaffRow()` автоматически добавляется новый ключ

### 15.2. QPainter state management

**Критически важно** вызывать `painter.save()` / `painter.restore()` при изменении кисти, пера, трансформаций. Без этого изменения состояния «протекают» на последующие операции отрисовки.

**Пример бага (исправлен):** Dot рисовался с `setBrush(Qt::black, SolidPattern)` без `save/restore`, из-за чего все последующие прямоугольники выделения становились чёрными закрашенными.

### 15.3. Флип нот

Ноты с `staffStep > 4` (выше средней линии) рисуются с вертикальным флипом (`painter.scale(1, -1)`) для изменения направления штиля. Это не применяется к Whole и Undefined нотам.

### 15.4. Переход между станами

При перемещении символа за правый край текущего стана:
- Если есть следующий стан → переход на него с `xSlot = kDefaultTrebleClefSlot + 1`
- Если нет → `addStaffRow()` создаёт новый стан

### 15.5. Точность hit-testing

Методы `*At()` в `Score` используют `slotTolerance` и `stepTolerance`. По умолчанию `slotTolerance=0`, `stepTolerance=0` или `1` — т.е. поиск достаточно строгий. При необходимости можно увеличить допуски.

### 15.6. YIN-детектор

- Работает только с моно-сигналом
- Порог `0.1` в `absoluteThreshold` — стандартное значение, можно настроить
- Дебаунсинг требует 3 последовательных detections одной ноты
- Не распознаёт аккорды (только одну ноту за раз)
- Игнорирует диезы/бемоли в `addNoteFromMidi()` — только натуральные ноты

### 15.7. Минимальный размер виджета

`minimumSizeHint()` рассчитывается динамически на основе `m_staffCount` и `m_spacingK`. При добавлении стана виджет автоматически ресайзится.

### 15.8. CMake и Qt

- `CMAKE_AUTOMOC`, `CMAKE_AUTORCC`, `CMAKE_AUTOUIC` включены
- Все `.h` файлы перечислены в `add_executable()` (нужно для AUTOMOC)
- Ресурсы подключаются через `symbols.qrc` с префиксом `:/`
