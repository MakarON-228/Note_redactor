cmake_minimum_required(VERSION 3.16)
project(NoteRedactor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg Multimedia)

add_executable(NoteRedactor
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/common/ToolType.h
    src/model/MusicSymbol.h
    src/model/BarLine.h
    src/model/GlyphSymbol.h
    src/model/Note.h
    src/model/Note.cpp
    src/model/Score.h
    src/model/Score.cpp
    src/model/TimeSignature.h
    src/view/SymbolPaletteWidget.h
    src/view/SymbolPaletteWidget.cpp
    src/view/StaffWidget.h
    src/view/StaffWidget.cpp
    src/audio/AudioRecorder.h
    src/audio/AudioRecorder.cpp
    src/audio/PitchDetector.h
    src/audio/PitchDetector.cpp
    resources/symbols.qrc
)

target_link_libraries(NoteRedactor PRIVATE Qt6::Widgets Qt6::Svg Qt6::Multimedia)
