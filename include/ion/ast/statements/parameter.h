#pragma once
#include <algorithm>
#include <optional>

#include "ion/token.h"
#include "ion/ast/node.h"

class Parameter final : public Statement
{
public:
    Token name;
    std::optional<ColonTypeClause*> colon_type;
    std::optional<EqualsValueClause*> equals_value;

    explicit Parameter(
        Token name,
        const std::optional<ColonTypeClause*> colon_type,
        const std::optional<EqualsValueClause*> equals_value)
        : name(std::move(name)),
          colon_type(colon_type),
          equals_value(equals_value)
    {
    }

    ~Parameter() override
    {
        if (colon_type.has_value())
            delete *colon_type;
        if (equals_value.has_value())
            delete *equals_value;
    }

    static statement_ptr_t create(
        Token name,
        const std::optional<ColonTypeClause*> colon_type,
        const std::optional<EqualsValueClause*> equals_value)
    {
        return std::make_unique<Parameter>(std::move(name), colon_type, equals_value);
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_parameter(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return name;
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
        const auto initializer_text = equals_value.has_value() ? equals_value.value()->get_text() : "";
        const auto colon_type_text = colon_type.has_value() ? colon_type.value()->get_text() : "";

        return name.get_text() + colon_type_text + initializer_text;
    }
};