#pragma once
#include "../abstract/expression.h"
#include "../../syntax/token.h"

class literal : public expression
{
public:
    token token;

    explicit literal(::token token)
        : token(std::move(token))
    {
    }
};
