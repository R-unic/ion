#pragma once
#include <algorithm>

#include "ion/token.h"
#include "ion/ast/node.h"

class Repeat final : public Statement
{
public:
    Token repeat_keyword, while_keyword;
    expression_ptr_t condition;
    statement_ptr_t statement;

    explicit Repeat(Token repeat_keyword, statement_ptr_t statement, Token while_keyword, expression_ptr_t condition)
        : repeat_keyword(std::move(repeat_keyword)),
          while_keyword(std::move(while_keyword)),
          condition(std::move(condition)),
          statement(std::move(statement))
    {
    }

    static statement_ptr_t create(Token repeat_keyword, statement_ptr_t statement, Token while_keyword, expression_ptr_t condition)
    {
        return std::make_unique<Repeat>(std::move(repeat_keyword), std::move(statement), std::move(while_keyword), std::move(condition));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_repeat(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return repeat_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return condition->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto separator = statement->is_block() ? ' ' : '\n';
        return std::string("repeat") + separator + statement->get_text() + separator + "while " + condition->get_text();
    }
};