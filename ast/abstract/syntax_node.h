#pragma once
#ifndef SYNTAX_NODE_H
#define SYNTAX_NODE_H

class literal;

class syntax_node_visitor {};

template <typename R>
class expression_visitor : public syntax_node_visitor
{
protected:
    ~expression_visitor() = default;

public:
    virtual R visit(literal literal) = 0;
};

template <typename R>
class statement_visitor : public syntax_node_visitor
{
protected:
    ~statement_visitor() = default;
};


class syntax_node
{
public:
    template <typename R>
    R accept(expression_visitor<R> visitor)
    {
        return visitor.visit(this);
    }

    template <typename R>
    R accept(statement_visitor<R> visitor)
    {
        return visitor.visit(this);
    }
};

#endif