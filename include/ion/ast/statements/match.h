#pragma once
#include <algorithm>

#include "ion/token.h"
#include "ion/ast/node.h"

class Match final : public Statement
{
public:
    Token match_keyword;
    expression_ptr_t expression;
    BracedStatementList* cases;

    explicit Match(Token match_keyword, expression_ptr_t expression, BracedStatementList* cases)
        : match_keyword(std::move(match_keyword)),
          expression(std::move(expression)),
          cases(cases)
    {
    }

    ~Match() override
    {
        delete cases;
    }

    static statement_ptr_t create(Token match_keyword, expression_ptr_t expression, BracedStatementList* members)
    {
        return std::make_unique<Match>(std::move(match_keyword), std::move(expression), members);
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_match(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return match_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return cases->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        return "match " + expression->get_text() + ' ' + cases->get_text();
    }
};