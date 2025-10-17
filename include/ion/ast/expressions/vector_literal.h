#pragma once
#include <string>

#include "ion/token.h"
#include "ion/ast/node.h"

class VectorLiteral final : public Expression
{
public:
    Token l_arrow, r_arrow;
    expression_ptr_t x, y, z;

    explicit VectorLiteral(Token l_arrow, Token r_arrow, expression_ptr_t x, expression_ptr_t y, expression_ptr_t z)
        : l_arrow(std::move(l_arrow)),
          r_arrow(std::move(r_arrow)),
          x(std::move(x)),
          y(std::move(y)),
          z(std::move(z))
    {
    }

    static expression_ptr_t create(Token l_arrow, Token r_arrow, expression_ptr_t x, expression_ptr_t y, expression_ptr_t z)
    {
        return std::make_unique<VectorLiteral>(std::move(l_arrow), std::move(r_arrow), std::move(x), std::move(y), std::move(z));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_vector_literal(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return l_arrow;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return r_arrow;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return '<' + x->get_text() + ", " + y->get_text() + ", " + z->get_text() + '>';
    }

    [[nodiscard]] bool is_literal() const override
    {
        return true;
    }
};