#pragma once
#include <unordered_map>

#include "token.h"

struct LexState : FileLocation
{
    FileLocation lexeme_start;
    std::vector<Token> tokens;
};

template <typename K, typename V>
static std::unordered_map<V, K> inverse_map(const std::unordered_map<K, V>& forward_map)
{
    std::unordered_map<V, K> map;
    for (auto& [lexeme, kind] : forward_map)
        map.emplace(kind, lexeme);
    
    return map;
}

const std::unordered_map<char, SyntaxKind> single_char_syntaxes = {
    {';', SyntaxKind::Semicolon},
    {',', SyntaxKind::Comma},
    {'.', SyntaxKind::Dot},
    {'(', SyntaxKind::LParen},
    {')', SyntaxKind::RParen},
    {'[', SyntaxKind::LBracket},
    {']', SyntaxKind::RBracket},
    {'{', SyntaxKind::LBrace},
    {'}', SyntaxKind::RBrace}
};
const auto single_char_lexemes = inverse_map(single_char_syntaxes);

const std::unordered_map<std::string, SyntaxKind> keyword_syntaxes = {
    {"let", SyntaxKind::LetKeyword},
    {"fn", SyntaxKind::FnKeyword},
    {"return", SyntaxKind::ReturnKeyword},
    {"break", SyntaxKind::BreakKeyword},
    {"continue", SyntaxKind::ContinueKeyword},
    {"if", SyntaxKind::IfKeyword},
    {"else", SyntaxKind::ElseKeyword},
    {"while", SyntaxKind::WhileKeyword},
    {"for", SyntaxKind::ForKeyword},
    {"instance", SyntaxKind::InstanceKeyword},
    {"event", SyntaxKind::EventKeyword},
    {"typeof", SyntaxKind::TypeOfKeyword},
    {"nameof", SyntaxKind::NameOfKeyword},
    {"type", SyntaxKind::TypeKeyword},
    {"interface", SyntaxKind::InterfaceKeyword},
    {"import", SyntaxKind::ImportKeyword},
    {"export", SyntaxKind::ExportKeyword},
    {"from", SyntaxKind::FromKeyword},
    {"true", SyntaxKind::TrueKeyword},
    {"false", SyntaxKind::FalseKeyword},
    {"null", SyntaxKind::NullKeyword}
};
const auto keyword_lexemes = inverse_map(keyword_syntaxes);

std::vector<Token> tokenize(const SourceFile*);
