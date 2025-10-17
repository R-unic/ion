#pragma once
#include <algorithm>

#include "ion/token.h"
#include "ion/ast/node.h"

class While final : public Statement
{
public:
    Token while_keyword;
    expression_ptr_t condition;
    statement_ptr_t statement;

    explicit While(Token while_keyword, expression_ptr_t condition, statement_ptr_t statement)
        : while_keyword(std::move(while_keyword)),
          condition(std::move(condition)),
          statement(std::move(statement))
    {
    }

    static statement_ptr_t create(Token while_keyword, expression_ptr_t condition, statement_ptr_t statement)
    {
        return std::make_unique<While>(std::move(while_keyword), std::move(condition), std::move(statement));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_while(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return while_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return statement->get_last_token();
    }

    [[nodiscard]] FileSpan get_span() const override
    {
        return create_span(while_keyword.span.start, statement->get_span().end);
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto separator = statement->is_block() ? ' ' : '\n';
        return "while " + condition->get_text() + separator + statement->get_text();
    }
};