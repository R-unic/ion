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
    {"instance", SyntaxKind::InstanceKeyword},
    {"event", SyntaxKind::EventKeyword},
    {"typeof", SyntaxKind::TypeOfKeyword},
    {"type", SyntaxKind::TypeKeyword},
    {"interface", SyntaxKind::InterfaceKeyword},
    {"import", SyntaxKind::ImportKeyword},
    {"from", SyntaxKind::FromKeyword},
    {"true", SyntaxKind::TrueKeyword},
    {"false", SyntaxKind::FalseKeyword},
    {"null", SyntaxKind::NullKeyword}
};

std::vector<Token> tokenize(const SourceFile&);
