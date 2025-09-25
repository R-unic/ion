#pragma once
#include "token.h"
#include "ast/node.h"

class MemberAccess final : public Expression
{
public:
    Token dot_token;
    expression_ptr_t expression;
    Token name;
    
    explicit MemberAccess(Token dot_token, expression_ptr_t expression, Token name)
        : dot_token(std::move(dot_token)),
        expression(std::move(expression)),
        name(std::move(name))
    {
    }
    
    static expression_ptr_t create(Token operator_token, expression_ptr_t expression, Token name)
    {
        return std::make_unique<MemberAccess>(std::move(operator_token), std::move(expression), std::move(name));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_member_access(*this);
    }
};