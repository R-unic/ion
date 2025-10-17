#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"
#include "unary_op.h"

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

    [[nodiscard]] Token get_first_token() const override
    {
        return operand->get_first_token();
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return operator_token;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return operand->get_text() + operator_token.get_text();
    }
};