#pragma once
#include <vector>

#include "../node.h"
#include "ion/token.h"
#include "ion/utility/ast.h"

struct TypeListClause final : SyntaxNode
{
    Token l_arrow, r_arrow;
    std::vector<type_ref_ptr_t> list;

    explicit TypeListClause(Token l_arrow, std::vector<type_ref_ptr_t> list, Token r_arrow)
        : l_arrow(std::move(l_arrow)),
          r_arrow(std::move(r_arrow)),
          list(std::move(list))
    {
    }

    TypeListClause(const TypeListClause&) = delete;
    TypeListClause& operator=(const TypeListClause&) = delete;

    [[nodiscard]] Token get_first_token() const override
    {
        return l_arrow;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return r_arrow;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return '<' + join_by(list, ", ") + '>';
    }
};