#pragma once
#include <optional>
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
    ImportKeyword,
    FromKeyword,
    TrueKeyword,
    FalseKeyword,
    NumberKeyword,
    StringKeyword,
    NullKeyword,
    BoolKeyword,
    VoidKeyword
};

struct Token {
    SyntaxKind kind;
    FileSpan span;
    std::optional<std::string> text;
};

inline std::string get_text(const Token& token)
{
    const auto start_position = token.span.start.position;
    return token.text.has_value() 
    ? token.text.value()
        : token.span.start.file.text.substr(start_position, token.span.end.position - start_position);
}

inline std::string format_token(const Token& token)
{
    return get_text(token) + " (" + std::to_string(static_cast<int>(token.kind)) + ')';
}