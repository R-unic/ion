#pragma once
#include "file_location.h"
#include "token.h"

struct LexState : FileLocation
{
    FileLocation lexeme_start;
    std::vector<Token, std::allocator<Token>>* tokens;
};

std::vector<Token, std::allocator<Token>> tokenize(const SourceFile&);
