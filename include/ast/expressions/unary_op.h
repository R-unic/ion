#pragma once
#include "token.h"
#include "ast/node.h"

class UnaryOp : public Expression
{
public:
    Token operator_token;
    expression_ptr_t operand;
    
    UnaryOp(Token operator_token, expression_ptr_t operand)
        : operator_token(std::move(operator_token)),
        operand(std::move(operand))
    {
    }
    
    static expression_ptr_t create(Token operator_token, expression_ptr_t operand)
    {
        return std::make_unique<UnaryOp>(std::move(operator_token), std::move(operand));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_unary_op(*this);
    }

    [[nodiscard]] FileSpan get_span() const override
    {
        return create_span(operator_token.span.start, operand->get_span().end);
    }
    
    [[nodiscard]] std::string get_text() const override
    {
        return operator_token.get_text() + operand->get_text();
    }
};