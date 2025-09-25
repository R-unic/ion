#pragma once
#include "token.h"
#include "ast/node.h"

class BinaryOp : public Expression
{
public:
    Token operator_token;
    expression_ptr_t left;
    expression_ptr_t right;
    
    explicit BinaryOp(Token operator_token, expression_ptr_t left, expression_ptr_t right)
        : operator_token(std::move(operator_token)), left(std::move(left)), right(std::move(right))
    {
    }
    
    static expression_ptr_t create(const Token& operator_token, expression_ptr_t left, expression_ptr_t right)
    {
        return std::make_unique<BinaryOp>(operator_token, std::move(left), std::move(right));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_binary_op(*this);
    }
};