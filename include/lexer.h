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
    {'.', SyntaxKind::Dot}
};

const std::unordered_map<std::string, SyntaxKind> keyword_syntaxes = {
    {"let", SyntaxKind::LetKeyword}
};

std::vector<Token> tokenize(const SourceFile&);
