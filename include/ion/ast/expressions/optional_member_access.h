#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"

class OptionalMemberAccess final : public Expression
{
public:
    Token token, question_token;
    expression_ptr_t expression;
    Token name;

    explicit OptionalMemberAccess(Token token, Token question_token, expression_ptr_t expression, Token name)
        : token(std::move(token)),
          question_token(std::move(question_token)),
          expression(std::move(expression)),
          name(std::move(name))
    {
    }

    static expression_ptr_t create(Token token, Token question_token, expression_ptr_t expression, Token name)
    {
        return std::make_unique<OptionalMemberAccess>(std::move(token), std::move(question_token), std::move(expression), std::move(name));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_optional_member_access(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return expression->get_first_token();
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return name;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return expression->get_text() + '?' + token.get_text() + name.get_text();
    }

    [[nodiscard]] bool is_name_of_target() const override
    {
        return true;
    }

    [[nodiscard]] bool is_assignment_target() const override
    {
        return true;
    }
};