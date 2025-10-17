#pragma once
#include <algorithm>

#include "ion/token.h"
#include "ion/ast/node.h"

class MatchElseCase final : public Statement
{
public:
    Token else_keyword, long_arrow;
    std::optional<Token> name;
    statement_ptr_t statement;

    explicit MatchElseCase(Token else_keyword, std::optional<Token> name, Token long_arrow, statement_ptr_t statement)
        : else_keyword(std::move(else_keyword)),
          long_arrow(std::move(long_arrow)),
          name(std::move(name)),
          statement(std::move(statement))
    {
    }

    static statement_ptr_t create(Token else_keyword, std::optional<Token> name, Token long_arrow, statement_ptr_t statement)
    {
        return std::make_unique<MatchElseCase>(std::move(else_keyword), std::move(name), std::move(long_arrow), std::move(statement));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_match_else_case(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return else_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return statement->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        return "else" + (name.has_value() ? ' ' + name->get_text() : "") + " -> " + statement->get_text();
    }
};