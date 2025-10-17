#pragma once
#include <algorithm>

#include "ion/token.h"
#include "ion/ast/node.h"

class EnumMember final : public NamedDeclaration
{
public:
    std::optional<EqualsValueClause*> equals_value;

    explicit EnumMember(Token name, const std::optional<EqualsValueClause*> equals_value)
        : NamedDeclaration(std::move(name)),
          equals_value(equals_value)
    {
    }

    ~EnumMember() override
    {
        if (equals_value.has_value())
            delete *equals_value;
    }

    static statement_ptr_t create(Token name, const std::optional<EqualsValueClause*> equals_value)
    {
        return std::make_unique<EnumMember>(std::move(name), equals_value);
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_enum_member(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return name;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return equals_value.has_value() ? equals_value.value()->get_last_token() : name;
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto equals_value_text = equals_value.has_value() ? equals_value.value()->get_text() : "";
        return name.get_text() + equals_value_text;
    }
};