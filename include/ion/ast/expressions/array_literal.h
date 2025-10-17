#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"
#include "ion/utility/ast.h"

class ArrayLiteral final : public Expression
{
public:
    Token l_bracket, r_bracket;
    std::vector<expression_ptr_t> elements;

    explicit ArrayLiteral(Token l_bracket, Token r_bracket, std::vector<expression_ptr_t> expressions)
        : l_bracket(std::move(l_bracket)),
          r_bracket(std::move(r_bracket)),
          elements(std::move(expressions))
    {
    }

    static expression_ptr_t create(Token l_bracket, Token r_bracket, std::vector<expression_ptr_t> expressions)
    {
        return std::make_unique<ArrayLiteral>(std::move(l_bracket), std::move(r_bracket), std::move(expressions));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_array_literal(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return l_bracket;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return r_bracket;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return '[' + join_by(elements, ", ") + ']';
    }

    [[nodiscard]] bool is_literal() const override
    {
        return true;
    }
};