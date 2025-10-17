#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"

struct ColonTypeClause final : SyntaxNode
{
    Token colon_token;
    type_ref_ptr_t type;

    explicit ColonTypeClause(Token colon_token, type_ref_ptr_t type)
        : colon_token(std::move(colon_token)),
          type(std::move(type))
    {
    }

    ColonTypeClause(const ColonTypeClause&) = delete;
    ColonTypeClause& operator=(const ColonTypeClause&) = delete;

    [[nodiscard]] Token get_first_token() const override
    {
        return colon_token;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return type->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        return ": " + type->get_text();
    }
};