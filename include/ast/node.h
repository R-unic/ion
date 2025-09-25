#pragma once
#include <memory>

#include "visitor.h"

class SyntaxNode
{
public:
    virtual ~SyntaxNode() = default;
};

class Expression : public SyntaxNode
{
public:
    virtual void accept(ExpressionVisitor<void>&) = 0;
};

class Statement : public SyntaxNode
{
public:
    virtual void accept(StatementVisitor<void>&) = 0;
};

using expression_ptr_t = std::unique_ptr<Expression>;
using statement_ptr_t = std::unique_ptr<Statement>;