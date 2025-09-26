#pragma once
#include <sstream>
#include <ranges>
#include <vector>

#include "token.h"
#include "utility.h"
#include "ast/node.h"

class Invocation : public Expression
{
public:
    Token l_paren, r_paren;
    expression_ptr_t callee;
    std::optional<Token> bang_token;
    std::vector<expression_ptr_t>* arguments;

    explicit Invocation(Token l_paren, Token r_paren, expression_ptr_t callee, std::optional<Token> bang_token, std::vector<expression_ptr_t>* arguments)
        : l_paren(std::move(l_paren)),
        r_paren(std::move(r_paren)),
        callee(std::move(callee)),
        bang_token(std::move(bang_token)),
        arguments(arguments)
    {
    }

    static expression_ptr_t create(Token l_paren, Token r_paren, expression_ptr_t callee, std::optional<Token> bang_token, std::vector<expression_ptr_t>* arguments)
    {
        return std::make_unique<Invocation>(std::move(l_paren), std::move(r_paren), std::move(callee), std::move(bang_token), arguments);
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_invocation(*this);
    }

    [[nodiscard]] FileSpan get_span() const override
    {
        return create_span(callee->get_span().start, r_paren.span.end);
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto args_string = join_by(*arguments, ", ");
        return callee->get_text() + (bang_token.has_value() ? "!" : "") + '(' + args_string + ')';
    }
};