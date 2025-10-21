#pragma once
#include <algorithm>
#include <optional>
#include <vector>

#include "../containers/type_list_clause.h"
#include "ion/token.h"
#include "../../utility/basic.h"
#include "ion/ast/node.h"

class For final : public Statement
{
public:
    Token for_keyword, colon_token;
    std::vector<Token> names;
    expression_ptr_t iterable;
    statement_ptr_t statement;

    explicit For(Token for_keyword, std::vector<Token> names, Token colon_token, expression_ptr_t iterable, statement_ptr_t statement)
        : for_keyword(std::move(for_keyword)),
          colon_token(std::move(colon_token)),
          names(std::move(names)),
          iterable(std::move(iterable)),
          statement(std::move(statement))
    {
    }

    static statement_ptr_t create(Token for_keyword, std::vector<Token> names, Token colon_token, expression_ptr_t iterable,
                                  statement_ptr_t statement)
    {
        return std::make_unique<For>(std::move(for_keyword), std::move(names), std::move(colon_token), std::move(iterable),
                                     std::move(statement));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_for(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return for_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return statement->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto separator = statement->is_block() ? ' ' : '\n';
        return "for " + join_by(names, ", ") + " : " + iterable->get_text() + separator + statement->get_text();
    }
};