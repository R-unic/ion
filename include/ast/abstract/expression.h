#pragma once
#include "syntax_node.h"
#include "../visitors.h"

class expression : public virtual syntax_node
{
    template <typename R>
    R accept(expression_visitor<R> visitor)
    {
        return visitor.visit(this);
    }
};
