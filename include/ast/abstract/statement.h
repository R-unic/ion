#pragma once
#include "syntax_node.h"
#include "../visitors.h"

class statement : public virtual syntax_node
{
    template <typename R>
    R accept(statement_visitor<R> visitor)
    {
        return visitor.visit(this);
    }
};
