#pragma once
#include <unordered_map>

#include "file_location.h"
#include "token.h"

struct LexState : FileLocation
{
    FileLocation lexeme_start;
    std::vector<Token>* tokens;
};

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
    {"import", SyntaxKind::ImportKeyword},
    {"from", SyntaxKind::FromKeyword},
    {"number", SyntaxKind::NumberKeyword},
    {"string", SyntaxKind::StringKeyword},
    {"bool", SyntaxKind::BoolKeyword},
    {"void", SyntaxKind::VoidKeyword},
};

std::vector<Token> tokenize(const SourceFile&);
