#pragma once
#include <optional>
#include <string>

#include "file_location.h"

enum class SyntaxKind : uint8_t
{
    Plus,
    PlusEquals,
    PlusPlus,
    Minus,
    MinusMinus,
    MinusEquals,
    Star,
    StarEquals,
    Slash,
    SlashEquals,
    Percent,
    PercentEquals,
    Carat,
    CaratEquals,
    AmpersandAmpersand,
    AmpersandAmpersandEquals,
    PipePipe,
    PipePipeEquals,
    Ampersand,
    AmpersandEquals,
    Pipe,
    PipeEquals,
    Tilde,
    TildeEquals,
    Bang,
    BangEquals,
    Equals,
    EqualsEquals,
    Semicolon,
    Comma,
    Dot,
    Colon,
    ColonColon,
    Question,
    QuestionQuestion,
    QuestionQuestionEquals,
    LParen,
    RParen,
    LBracket,
    RBracket,
    LBrace,
    RBrace,
    LArrow,
    LArrowEquals,
    LArrowLArrow,
    LArrowLArrowEquals,
    RArrow,
    RArrowEquals,
    RArrowRArrow,
    RArrowRArrowEquals,
    RArrowRArrowRArrow,
    RArrowRArrowRArrowEquals,

    Identifier,
    NumberLiteral,
    StringLiteral,

    LetKeyword,
    FnKeyword,
    ReturnKeyword,
    BreakKeyword,
    ContinueKeyword,
    IfKeyword,
    ElseKeyword,
    WhileKeyword,
    ForKeyword,
    InstanceKeyword,
    EventKeyword,
    TypeOfKeyword,
    NameOfKeyword,
    TypeKeyword,
    InterfaceKeyword,
    ImportKeyword,
    ExportKeyword,
    FromKeyword,
    TrueKeyword,
    FalseKeyword,
    NullKeyword
};

struct Token {
    SyntaxKind kind;
    FileSpan span;
    std::optional<std::string> text = std::nullopt;

    [[nodiscard]] std::string get_text() const;
    std::string format();
};