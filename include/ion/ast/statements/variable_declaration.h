#pragma once
#include <algorithm>
#include <optional>

#include "../containers/colon_type_clause.h"
#include "../containers/equals_value_clause.h"
#include "ion/token.h"
#include "ion/ast/node.h"

class VariableDeclaration final : public NamedDeclaration
{
public:
    Token let_keyword;
    std::optional<Token> const_keyword;
    std::optional<ColonTypeClause*> colon_type;
    std::optional<EqualsValueClause*> equals_value;

    explicit VariableDeclaration(
        Token let_keyword,
        std::optional<Token> const_keyword,
        Token name,
        const std::optional<ColonTypeClause*> colon_type,
        const std::optional<EqualsValueClause*> equals_value)
        : NamedDeclaration(std::move(name)),
          let_keyword(std::move(let_keyword)),
          const_keyword(std::move(const_keyword)),
          colon_type(colon_type),
          equals_value(equals_value)
    {
    }

    ~VariableDeclaration() override
    {
        if (colon_type.has_value())
            delete *colon_type;
        if (equals_value.has_value())
            delete *equals_value;
    }

    static statement_ptr_t create(
        Token let_keyword,
        std::optional<Token> const_keyword,
        Token name,
        const std::optional<ColonTypeClause*> colon_type,
        const std::optional<EqualsValueClause*> equals_value)
    {
        return std::make_unique<VariableDeclaration>(std::move(let_keyword), std::move(const_keyword), std::move(name),
                                                     colon_type, equals_value);
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_variable_declaration(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return let_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return equals_value.has_value()
                   ? equals_value.value()->get_last_token()
                   : colon_type.has_value()
                         ? colon_type.value()->get_last_token()
                         : name;
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto equals_value_text = equals_value.has_value() ? equals_value.value()->get_text() : "";
        const auto colon_type_text = colon_type.has_value() ? colon_type.value()->get_text() : "";

        return "let " + name.get_text() + colon_type_text + equals_value_text;
    }
};