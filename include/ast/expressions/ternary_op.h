#pragma once
#include "token.h"
#include "ast/node.h"

class TernaryOp final : public Expression
{
public:
    Token question_token, colon_token;
    expression_ptr_t condition, when_true, when_false;
    
    explicit TernaryOp(Token question_token, Token colon_token, expression_ptr_t condition, expression_ptr_t when_true, expression_ptr_t when_false)
        : question_token(std::move(question_token)),
        colon_token(std::move(colon_token)),
        condition(std::move(condition)),
        when_true(std::move(when_true)),
        when_false(std::move(when_false))
    {
    }
    
    static expression_ptr_t create(Token question_token, Token colon_token, expression_ptr_t condition, expression_ptr_t when_true, expression_ptr_t when_false)
    {
        return std::make_unique<TernaryOp>(std::move(question_token), std::move(colon_token), std::move(condition), std::move(when_true), std::move(when_false));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_ternary_op(*this);
    }

    [[nodiscard]] FileSpan get_span() const override
    {
        return create_span(condition->get_span().start, when_false->get_span().end);
    }
    
    [[nodiscard]] std::string get_text() const override
    {
        return condition->get_text() + " ? " + when_true->get_text() + " : " + when_false->get_text();
    }
};