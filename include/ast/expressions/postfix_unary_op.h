#pragma once
#include "token.h"
#include "unary_op.h"
#include "ast/node.h"

class PostfixUnaryOp final : public UnaryOp
{
public:
    PostfixUnaryOp(Token operator_token, expression_ptr_t operand)
        : UnaryOp(std::move(operator_token), std::move(operand))
    {
    }
    
    static expression_ptr_t create(Token operator_token, expression_ptr_t operand)
    {
        return std::make_unique<PostfixUnaryOp>(std::move(operator_token), std::move(operand));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_postfix_unary_op(*this);
    }
    
    [[nodiscard]] FileSpan get_span() const override
    {
        return create_span(operand->get_span().start, operator_token.span.end);
    }
    
    [[nodiscard]] std::string get_text() const override
    {
        return operand->get_text() + operator_token.get_text();
    }
};