#pragma once

class ExpressionStatement;
class Literal;
class BinaryOp;

template <typename R>
struct ExpressionVisitor
{
    virtual R visit_literal(const Literal&) = 0;
    virtual R visit_binary_op(const BinaryOp&) = 0;
    
    virtual ~ExpressionVisitor() = default;
};

template <typename R>
struct StatementVisitor
{
    virtual R visit_expression_statement(const ExpressionStatement&) = 0;

    virtual ~StatementVisitor() = default;
};