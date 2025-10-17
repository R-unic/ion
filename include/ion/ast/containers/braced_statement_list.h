#pragma once
#include <vector>

#include "ion/token.h"
#include "ion/ast/node.h"
#include "ion/utility/ast.h"

struct BracedStatementList final : SyntaxNode
{
    Token l_brace, r_brace;
    std::vector<statement_ptr_t> statements;

    BracedStatementList(Token l_brace, std::vector<statement_ptr_t> statements, Token r_brace)
        : l_brace(std::move(l_brace)),
          r_brace(std::move(r_brace)),
          statements(std::move(statements))
    {
    }

    BracedStatementList(const BracedStatementList&) = delete;
    BracedStatementList& operator=(const BracedStatementList&) = delete;

    [[nodiscard]] Token get_first_token() const override
    {
        return l_brace;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return r_brace;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return "{\n" + join_by(statements, "\n") + "\n}";
    }
};