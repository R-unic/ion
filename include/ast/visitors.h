#pragma once
#include "expressions/literal.h"

class syntax_node_visitor {
protected:
    ~syntax_node_visitor() = default;
};

template <typename R>
class expression_visitor : public virtual syntax_node_visitor
{
protected:
    ~expression_visitor() = default;

public:
    virtual R visit(literal literal) = 0;
};

template <typename R>
class statement_visitor : public virtual syntax_node_visitor
{
protected:
    ~statement_visitor() = default;
};