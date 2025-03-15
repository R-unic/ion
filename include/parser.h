#pragma once
#include "ast/expressions.h"
#include "syntax/token_stream.h"

class parser
{
    token_stream tokens;
    
public:
    explicit parser(token_stream tokens)
        : tokens(std::move(tokens))
    {
    }

    expression parse();
    expression parse_expression();
    expression parse_primary();
};
