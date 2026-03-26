#pragma once

class MusicSymbol {
public:
    enum class SymbolType {
        Note,
        TrebleClef,
        Rest,
        Sharp,
        Flat,
        Beccare,
        BarLine,
        TimeSignature
    };

    explicit MusicSymbol(SymbolType symbolType) : m_type(symbolType) {}
    virtual ~MusicSymbol() = default;

    SymbolType type() const { return m_type; }

private:
    SymbolType m_type;
};
