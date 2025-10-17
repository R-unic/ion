#pragma once
#include <optional>
#include <vector>

#include "file_location.h"

enum class SyntaxKind : unsigned char
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
    SlashSlash,
    SlashSlashEquals,
    Percent,
    PercentEquals,
    Caret,
    CaretEquals,
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
    DotDot,
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
    LongArrow,
    Hashtag,
    At,

    Identifier,
    NumberLiteral,
    StringLiteral,
    InterpolatedStringPart,
    InterpolationStart,
    InterpolationEnd,

    LetKeyword,
    ConstKeyword,
    FnKeyword,
    ReturnKeyword,
    BreakKeyword,
    ContinueKeyword,
    IfKeyword,
    ElseKeyword,
    WhileKeyword,
    RepeatKeyword,
    ForKeyword,
    InstanceKeyword,
    CloneKeyword,
    EventKeyword,
    EnumKeyword,
    TypeOfKeyword,
    NameOfKeyword,
    TypeKeyword,
    InterfaceKeyword,
    AsyncKeyword,
    AwaitKeyword,
    AfterKeyword,
    EveryKeyword,
    MatchKeyword,
    ImportKeyword,
    ExportKeyword,
    FromKeyword,
    TrueKeyword,
    FalseKeyword,
    NullKeyword,
    RgbKeyword,
    HsvKeyword
};

struct Token
{
    SyntaxKind kind {};
    FileSpan span;
    std::optional<std::string> text;

    [[nodiscard]] bool is_kind(SyntaxKind) const;
    [[nodiscard]] std::string get_text() const;
    [[nodiscard]] std::string format() const;
    [[nodiscard]] std::vector<Token> split(const std::vector<SyntaxKind>&) const;
};