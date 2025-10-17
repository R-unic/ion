#pragma once
#include <algorithm>

#include "ion/token.h"
#include "ion/ast/node.h"

class Every final : public Statement
{
public:
    Token every_keyword;
    expression_ptr_t time_expression;
    std::optional<Token> while_keyword;
    std::optional<expression_ptr_t> condition;
    statement_ptr_t statement;

    explicit Every(Token every_keyword, expression_ptr_t time_expression, std::optional<Token> while_keyword,
                   std::optional<expression_ptr_t> condition, statement_ptr_t statement)
        : every_keyword(std::move(every_keyword)),
          time_expression(std::move(time_expression)),
          while_keyword(std::move(while_keyword)),
          condition(std::move(condition)),
          statement(std::move(statement))
    {
    }

    static statement_ptr_t create(Token every_keyword, expression_ptr_t time_expression, std::optional<Token> while_keyword,
                                  std::optional<expression_ptr_t> condition, statement_ptr_t statement)
    {
        return std::make_unique<Every>(std::move(every_keyword), std::move(time_expression), std::move(while_keyword),
                                       std::move(condition), std::move(statement));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_every(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return every_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return statement->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto separator = statement->is_block() ? ' ' : '\n';
        const auto condition_text = condition.has_value() ? " while " + condition.value()->get_text() : "";
        return "every " + time_expression->get_text() + condition_text + separator + statement->get_text();
    }
};