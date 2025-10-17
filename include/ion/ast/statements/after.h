#pragma once
#include <algorithm>

#include "ion/token.h"
#include "ion/ast/node.h"

class After final : public Statement
{
public:
    Token after_keyword;
    expression_ptr_t time_expression;
    statement_ptr_t statement;

    explicit After(Token after_keyword, expression_ptr_t time_expression, statement_ptr_t statement)
        : after_keyword(std::move(after_keyword)),
          time_expression(std::move(time_expression)),
          statement(std::move(statement))
    {
    }

    static statement_ptr_t create(Token after_keyword, expression_ptr_t time_expression, statement_ptr_t statement)
    {
        return std::make_unique<After>(std::move(after_keyword), std::move(time_expression), std::move(statement));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_after(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return after_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return statement->get_last_token();
    }

    [[nodiscard]] FileSpan get_span() const override
    {
        return create_span(after_keyword.span.start, statement->get_span().end);
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto separator = statement->is_block() ? ' ' : '\n';
        return "after " + time_expression->get_text() + separator + statement->get_text();
    }
};