#pragma once
#include <format>

#include "file_location.h"
#include "ast/source_file.h"

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
    Dot,
    Colon,
    ColonColon,
    Question,
    QuestionQuestion,
    QuestionQuestionEquals,

    Identifier,
    NumberLiteral,
};

struct Token {
    SyntaxKind kind;
    FileSpan span;
};

inline std::string get_text(const Token& token)
{
    const auto start_position = token.span.start.position;
    return token.span.start.file.text.substr(start_position, token.span.end.position - start_position);
}