#pragma once
#include <algorithm>

#include "ion/token.h"
#include "ion/ast/node.h"

class NamedDeclaration : public Statement
{
public:
    Token name;

    explicit NamedDeclaration(Token name)
        : name(std::move(name))
    {
    }
};