#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"
#include "binary_op.h"

class AssignmentOp final : public BinaryOp
{
public:
    explicit AssignmentOp(Token operator_token, expression_ptr_t left, expression_ptr_t right)
        : BinaryOp(std::move(operator_token), std::move(left), std::move(right))
    {
    }

    static expression_ptr_t create(Token operator_token, expression_ptr_t left, expression_ptr_t right)
    {
        return std::make_unique<AssignmentOp>(std::move(operator_token), std::move(left), std::move(right));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_assignment_op(*this);
    }

    [[nodiscard]] bool is_assignment() const override
    {
        return true;
    }
};