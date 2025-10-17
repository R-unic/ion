#pragma once
#include <algorithm>

#include "ion/token.h"
#include "ion/ast/node.h"

class MatchCase final : public Statement
{
public:
    std::vector<expression_ptr_t> comparands;
    Token long_arrow;
    statement_ptr_t statement;

    explicit MatchCase(std::vector<expression_ptr_t> comparands, Token long_arrow, statement_ptr_t statement)
        : comparands(std::move(comparands)),
          long_arrow(std::move(long_arrow)),
          statement(std::move(statement))
    {
    }

    static statement_ptr_t create(std::vector<expression_ptr_t> comparands, Token long_arrow, statement_ptr_t statement)
    {
        return std::make_unique<MatchCase>(std::move(comparands), std::move(long_arrow), std::move(statement));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_match_case(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return comparands.front()->get_first_token();
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return statement->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        return join_by(comparands, ", ") + " -> " + statement->get_text();
    }
};