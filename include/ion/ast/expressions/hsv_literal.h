#pragma once
#include <string>

#include "ion/token.h"
#include "ion/ast/node.h"

class HsvLiteral final : public Expression
{
public:
    Token hsv_keyword, l_arrow, r_arrow;
    expression_ptr_t h, s, v;

    explicit HsvLiteral(Token hsv_keyword, Token l_arrow, Token r_arrow, expression_ptr_t h, expression_ptr_t s, expression_ptr_t v)
        : hsv_keyword(std::move(hsv_keyword)),
          l_arrow(std::move(l_arrow)),
          r_arrow(std::move(r_arrow)),
          h(std::move(h)),
          s(std::move(s)),
          v(std::move(v))
    {
    }

    static expression_ptr_t create(Token hsv_keyword, Token l_arrow, Token r_arrow,
                                   expression_ptr_t h, expression_ptr_t s, expression_ptr_t v)
    {
        return std::make_unique<HsvLiteral>(std::move(hsv_keyword), std::move(l_arrow), std::move(r_arrow),
                                            std::move(h), std::move(s), std::move(v));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_hsv_literal(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return hsv_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return r_arrow;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return "hsv<" + h->get_text() + ", " + s->get_text() + ", " + v->get_text() + '>';
    }

    [[nodiscard]] bool is_literal() const override
    {
        return true;
    }
};