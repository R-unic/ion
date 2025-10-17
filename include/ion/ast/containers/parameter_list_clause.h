#pragma once
#include <vector>

#include "ion/token.h"
#include "ion/ast/node.h"
#include "ion/utility/ast.h"

struct ParameterListClause final : SyntaxNode
{
    Token l_paren, r_paren;
    std::vector<statement_ptr_t> list;

    explicit ParameterListClause(Token l_paren, std::vector<statement_ptr_t> list, Token r_paren)
        : l_paren(std::move(l_paren)),
          r_paren(std::move(r_paren)),
          list(std::move(list))
    {
    }

    ParameterListClause(const ParameterListClause&) = delete;
    ParameterListClause& operator=(const ParameterListClause&) = delete;

    [[nodiscard]] Token get_first_token() const override
    {
        return l_paren;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return r_paren;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return '(' + join_by(list, ", ") + ')';
    }
};