#pragma once
#include "../abstract/expression.h"
#include "../../syntax/token.h"

class literal : public virtual expression
{
public:
    token token;

    explicit literal(::token token)
        : token(std::move(token))
    {
    }
};
