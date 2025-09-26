#pragma once
#include <memory>

#include "file_location.h"
#include "visitor_fwd.h"

class SyntaxNode
{
public:
    [[nodiscard]] virtual FileSpan get_span() const = 0;
    [[nodiscard]] virtual std::string get_text() const = 0;
    virtual ~SyntaxNode() = default;
};

class Expression : public SyntaxNode
{
public:
    virtual void accept(ExpressionVisitor<void>&) = 0;
    [[nodiscard]] virtual bool is_assignment_target() const { return false; }
};

class Statement : public SyntaxNode
{
public:
    virtual void accept(StatementVisitor<void>&) = 0;
    [[nodiscard]] virtual bool is_block() const { return false; }
};

using node_ptr_t = std::unique_ptr<SyntaxNode>;
using expression_ptr_t = std::unique_ptr<Expression>;
using statement_ptr_t = std::unique_ptr<Statement>;