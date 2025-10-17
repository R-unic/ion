#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"

class TypeParameterRef final : public TypeRef
{
public:
    Token name;
    std::optional<Token> colon_token, equals_token;
    std::optional<type_ref_ptr_t> base_type, default_type;

    explicit TypeParameterRef(Token name, std::optional<Token> colon_token, std::optional<type_ref_ptr_t> base_type,
                              std::optional<Token> equals_token, std::optional<type_ref_ptr_t> default_type)
        : name(std::move(name)),
          colon_token(std::move(colon_token)),
          equals_token(std::move(equals_token)),
          base_type(std::move(base_type)),
          default_type(std::move(default_type))
    {
    }

    static type_ref_ptr_t create(Token name, std::optional<Token> colon_token, std::optional<type_ref_ptr_t> base_type,
                                 std::optional<Token> equals_token, std::optional<type_ref_ptr_t> default_type)
    {
        return std::make_unique<TypeParameterRef>(std::move(name), std::move(colon_token), std::move(base_type),
                                                  std::move(equals_token), std::move(default_type));
    }

    void accept(TypeRefVisitor<void>& visitor) override
    {
        return visitor.visit_type_parameter(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return name;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return base_type.has_value() ? base_type.value()->get_last_token() : name;
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto base_type_text = base_type.has_value() ? ": " + base_type.value()->get_text() : "";
        return name.get_text() + base_type_text;
    }
};