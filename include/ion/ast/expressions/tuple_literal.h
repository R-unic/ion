#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"
#include "ion/utility/ast.h"

class TupleLiteral final : public Expression
{
public:
    Token l_paren, r_paren;
    std::vector<expression_ptr_t> elements;

    explicit TupleLiteral(Token l_paren, Token r_paren, std::vector<expression_ptr_t> expressions)
        : l_paren(std::move(l_paren)),
          r_paren(std::move(r_paren)),
          elements(std::move(expressions))
    {
    }

    static expression_ptr_t create(Token l_paren, Token r_paren, std::vector<expression_ptr_t> expressions)
    {
        return std::make_unique<TupleLiteral>(std::move(l_paren), std::move(r_paren), std::move(expressions));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_tuple_literal(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return l_paren;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return r_paren;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return '(' + join_by(elements, ", ") + ')';
    }

    [[nodiscard]] bool is_literal() const override
    {
        return true;
    }
};