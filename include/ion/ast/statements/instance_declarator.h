#pragma once
#include <algorithm>

#include "named_declaration.h"
#include "ion/token.h"

class InstanceDeclarator : public NamedDeclaration
{
protected:
    explicit InstanceDeclarator(Token name)
        : NamedDeclaration(std::move(name))
    {
    }
};