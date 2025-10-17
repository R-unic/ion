#pragma once
#include <string>

#include "ion/token.h"
#include "ion/ast/node.h"

class RgbLiteral final : public Expression
{
public:
    Token rgb_keyword, l_arrow, r_arrow;
    expression_ptr_t r, g, b;

    explicit RgbLiteral(Token rgb_keyword, Token l_arrow, Token r_arrow, expression_ptr_t r, expression_ptr_t g, expression_ptr_t b)
        : rgb_keyword(std::move(rgb_keyword)),
          l_arrow(std::move(l_arrow)),
          r_arrow(std::move(r_arrow)),
          r(std::move(r)),
          g(std::move(g)),
          b(std::move(b))
    {
    }

    static expression_ptr_t create(Token rgb_keyword, Token l_arrow, Token r_arrow,
                                   expression_ptr_t r, expression_ptr_t g, expression_ptr_t b)
    {
        return std::make_unique<RgbLiteral>(std::move(rgb_keyword), std::move(l_arrow), std::move(r_arrow),
                                            std::move(r), std::move(g), std::move(b));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_rgb_literal(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return rgb_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return r_arrow;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return "rgb<" + r->get_text() + ", " + g->get_text() + ", " + b->get_text() + '>';
    }

    [[nodiscard]] bool is_literal() const override
    {
        return true;
    }
};