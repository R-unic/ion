#pragma once
#include <memory>
#include <vector>

#include "token.h"

struct ParseState
{
    int position = 0;
    const SourceFile* file;
    std::vector<Token> tokens;
};

expression_ptr_t parse(const SourceFile*);