#pragma once

#include "token.h"
#include "utility/basic.h"
#include "utility/lexer.h"

struct OperatorRule
{
    SyntaxKind base;
    std::vector<std::pair<std::string, SyntaxKind>> continuations;
};

static const std::unordered_map<char, OperatorRule> operator_rules = {
    {
        '+', {
            SyntaxKind::Plus,
            {
                { "++", SyntaxKind::PlusPlus },
                { "+=", SyntaxKind::PlusEquals }
            }
        }
    },
    {
        '-', {
            SyntaxKind::Minus,
            {
                { "--", SyntaxKind::MinusMinus },
                { "-=", SyntaxKind::MinusEquals },
                { "->", SyntaxKind::LongArrow },
            }
        }
    },
    { '*', { SyntaxKind::Star, { { "*=", SyntaxKind::StarEquals } } } },
    {
        '/', {
            SyntaxKind::Slash,
            {
                { "/=", SyntaxKind::SlashEquals },
                { "//=", SyntaxKind::SlashSlashEquals },
                { "//", SyntaxKind::SlashSlash }
            }
        }
    },
    { '^', { SyntaxKind::Caret, { { "^=", SyntaxKind::CaretEquals } } } },
    { '%', { SyntaxKind::Percent, { { "%=", SyntaxKind::PercentEquals } } } },
    {
        '&', {
            SyntaxKind::Ampersand,
            {
                { "&=", SyntaxKind::AmpersandEquals },
                { "&&=", SyntaxKind::AmpersandAmpersandEquals },
                { "&&", SyntaxKind::AmpersandAmpersand }
            }
        }
    },
    {
        '|', {
            SyntaxKind::Pipe,
            {
                { "|=", SyntaxKind::PipeEquals },
                { "||=", SyntaxKind::PipePipeEquals },
                { "||", SyntaxKind::PipePipe }
            }
        }
    },
    { '~', { SyntaxKind::Tilde, { { "~=", SyntaxKind::TildeEquals } } } },
    {
        '<', {
            SyntaxKind::LArrow,
            {
                { "<=", SyntaxKind::LArrowEquals },
                { "<<=", SyntaxKind::LArrowLArrowEquals },
                { "<<", SyntaxKind::LArrowLArrow }
            }
        }
    },
    {
        '>', {
            SyntaxKind::RArrow,
            {
                { ">=", SyntaxKind::RArrowEquals },
                { ">>>=", SyntaxKind::RArrowRArrowRArrowEquals },
                { ">>>", SyntaxKind::RArrowRArrowRArrow },
                { ">>=", SyntaxKind::RArrowRArrowEquals },
                { ">>", SyntaxKind::RArrowRArrow }
            }
        }
    },
    { '!', { SyntaxKind::Bang, { { "!=", SyntaxKind::BangEquals } } } },
    { '=', { SyntaxKind::Equals, { { "==", SyntaxKind::EqualsEquals } } } },
    {
        '\?', {
            SyntaxKind::Question,
            {
                { "\?\?=", SyntaxKind::QuestionQuestionEquals },
                { "\?\?", SyntaxKind::QuestionQuestion },
            }
        }
    },
    { '.', { SyntaxKind::Dot, { { "..", SyntaxKind::DotDot } } } },
    { ':', { SyntaxKind::Colon, { { "::", SyntaxKind::ColonColon } } } },
};

const std::unordered_map<char, SyntaxKind> single_character_syntaxes = {
    { ';', SyntaxKind::Semicolon },
    { ',', SyntaxKind::Comma },
    { '@', SyntaxKind::At },
    { '#', SyntaxKind::Hashtag },
    { '(', SyntaxKind::LParen },
    { ')', SyntaxKind::RParen },
    { '[', SyntaxKind::LBracket },
    { ']', SyntaxKind::RBracket },
    { '{', SyntaxKind::LBrace },
    { '}', SyntaxKind::RBrace }
};

const auto single_character_lexemes = inverse_map(single_character_syntaxes);

const std::unordered_map<std::string, SyntaxKind> keyword_syntaxes = {
    { "let", SyntaxKind::LetKeyword },
    { "const", SyntaxKind::ConstKeyword },
    { "fn", SyntaxKind::FnKeyword },
    { "return", SyntaxKind::ReturnKeyword },
    { "break", SyntaxKind::BreakKeyword },
    { "continue", SyntaxKind::ContinueKeyword },
    { "if", SyntaxKind::IfKeyword },
    { "else", SyntaxKind::ElseKeyword },
    { "while", SyntaxKind::WhileKeyword },
    { "repeat", SyntaxKind::RepeatKeyword },
    { "for", SyntaxKind::ForKeyword },
    { "instance", SyntaxKind::InstanceKeyword },
    { "clone", SyntaxKind::CloneKeyword },
    { "event", SyntaxKind::EventKeyword },
    { "enum", SyntaxKind::EnumKeyword },
    { "typeof", SyntaxKind::TypeOfKeyword },
    { "nameof", SyntaxKind::NameOfKeyword },
    { "type", SyntaxKind::TypeKeyword },
    { "interface", SyntaxKind::InterfaceKeyword },
    { "async", SyntaxKind::AsyncKeyword },
    { "await", SyntaxKind::AwaitKeyword },
    { "after", SyntaxKind::AfterKeyword },
    { "every", SyntaxKind::EveryKeyword },
    { "match", SyntaxKind::MatchKeyword },
    { "import", SyntaxKind::ImportKeyword },
    { "export", SyntaxKind::ExportKeyword },
    { "from", SyntaxKind::FromKeyword },
    { "true", SyntaxKind::TrueKeyword },
    { "false", SyntaxKind::FalseKeyword },
    { "null", SyntaxKind::NullKeyword },
    { "rgb", SyntaxKind::RgbKeyword },
    { "hsv", SyntaxKind::HsvKeyword }
};

const auto keyword_lexemes = inverse_map(keyword_syntaxes);

static void lex(LexState&);
std::vector<Token> tokenize(const SourceFile&);

inline std::optional<std::string> syntax_to_string(const SyntaxKind kind)
{
    if (keyword_lexemes.contains(kind))
        return keyword_lexemes.at(kind);

    if (single_character_lexemes.contains(kind))
        return std::string(1, single_character_lexemes.at(kind));

    return std::nullopt;
}