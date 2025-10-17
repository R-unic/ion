#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"
#include "ion/utility/ast.h"

class InterpolatedString final : public Expression
{
public:
    std::vector<Token> parts;
    std::vector<expression_ptr_t> interpolations;

    explicit InterpolatedString(std::vector<Token> parts, std::vector<expression_ptr_t> interpolations)
        : parts(std::move(parts)),
          interpolations(std::move(interpolations))
    {
    }

    static expression_ptr_t create(std::vector<Token> parts, std::vector<expression_ptr_t> interpolations)
    {
        return std::make_unique<InterpolatedString>(std::move(parts), std::move(interpolations));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_interpolated_string(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return parts.front();
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return parts.back();
    }

    [[nodiscard]] std::string get_text() const override
    {
        auto i = 0;
        std::ostringstream oss;

        oss << parts[i++].get_text();
        while (i < interpolations.size())
        {
            const auto interpolation_text = interpolations[i]->get_text();
            const auto part_text = parts[i++].get_text();
            oss << "#{" << interpolation_text << "}" << part_text;
        }

        return '"' + oss.str() + '"';
    }

    [[nodiscard]] bool is_primitive_literal() const override
    {
        return true;
    }

    [[nodiscard]] bool is_literal() const override
    {
        return true;
    }
};