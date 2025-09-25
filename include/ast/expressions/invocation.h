#pragma once
#include "token.h"
#include "ast/node.h"

class Invocation : public Expression
{
public:
    Token l_paren, r_paren;
    expression_ptr_t callee;
    std::optional<Token> bang_token;
    std::vector<expression_ptr_t> arguments;
    
    explicit Invocation(Token l_paren, Token r_paren, expression_ptr_t callee, std::vector<expression_ptr_t> arguments)
        : l_paren(std::move(l_paren)),
        r_paren(std::move(r_paren)),
        callee(std::move(callee)),
        arguments(std::move(arguments))
    {
    }
    
    static expression_ptr_t create(Token l_paren, Token r_paren, expression_ptr_t callee, std::vector<expression_ptr_t> arguments)
    {
        return std::make_unique<Invocation>(std::move(l_paren), std::move(r_paren), std::move(callee), std::move(arguments));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_invocation(*this);
    }
};