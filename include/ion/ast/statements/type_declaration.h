#pragma once
#include <algorithm>
#include <optional>

#include "named_declaration.h"
#include "../containers/type_list_clause.h"
#include "ion/token.h"
#include "ion/ast/node.h"

class TypeDeclaration final : public NamedDeclaration
{
public:
    Token type_keyword, equals_token;
    std::optional<TypeListClause*> type_parameters;
    type_ref_ptr_t type;

    explicit TypeDeclaration(
        Token type_keyword,
        Token name,
        const std::optional<TypeListClause*> type_parameters,
        Token equals_token,
        type_ref_ptr_t type)
        : NamedDeclaration(std::move(name)),
          type_keyword(std::move(type_keyword)),
          equals_token(std::move(equals_token)),
          type_parameters(type_parameters),
          type(std::move(type))
    {
    }

    ~TypeDeclaration() override
    {
        if (type_parameters.has_value())
            delete *type_parameters;
    }

    static statement_ptr_t create(
        Token type_keyword,
        Token name,
        const std::optional<TypeListClause*> type_parameters,
        Token equals_token,
        type_ref_ptr_t type)
    {
        return std::make_unique<TypeDeclaration>(std::move(type_keyword), std::move(name), type_parameters,
                                                 std::move(equals_token), std::move(type));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_type_declaration(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return type_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return type->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto type_parameters_text = type_parameters.has_value() ? type_parameters.value()->get_text() : "";
        return "type " + name.get_text() + type_parameters_text + " = " + type->get_text();
    }
};