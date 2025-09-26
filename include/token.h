#pragma once
#include <optional>
#include <string>

#include "source_file.h"

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

struct FileLocation
{
    int position = 0;
    int line = 1;
    int column = 0;
    const SourceFile* file;
};

struct FileSpan
{
    FileLocation start;
    FileLocation end;
};

struct Token {
    SyntaxKind kind;
    FileSpan span;
    std::optional<std::string> text = std::nullopt;
};

std::string format_location(const FileLocation&, bool = true);
FileLocation get_start_location(const SourceFile*);
FileSpan create_span(const FileLocation&, const FileLocation&);
std::string get_text(const Token&);
std::string format_token(const Token&);