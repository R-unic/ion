#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"

class TypeNameRef final : public TypeRef
{
public:
    Token name;
    std::optional<TypeListClause*> type_arguments;

    explicit TypeNameRef(Token name, const std::optional<TypeListClause*> type_arguments)
        : name(std::move(name)),
          type_arguments(type_arguments)
    {
    }

    ~TypeNameRef() override
    {
        if (type_arguments.has_value())
            delete *type_arguments;
    }

    static type_ref_ptr_t create(Token name, const std::optional<TypeListClause*> type_arguments)
    {
        return std::make_unique<TypeNameRef>(std::move(name), type_arguments);
    }

    void accept(TypeRefVisitor<void>& visitor) override
    {
        return visitor.visit_type_name(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return name;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return type_arguments.has_value() ? type_arguments.value()->get_last_token() : name;
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto type_arguments_text = type_arguments.has_value() ? type_arguments.value()->get_text() : "";
        return name.get_text() + type_arguments_text;
    }
};