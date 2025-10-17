#pragma once
#include "ion/ast/node.h"
#include "ion/utility/basic.h"

class LiteralTypeRef final : public TypeRef
{
public:
    Token token;
    primitive_value_t value;

    explicit LiteralTypeRef(Token token, primitive_value_t value)
        : token(std::move(token)),
          value(std::move(value))
    {
    }

    static type_ref_ptr_t create(Token token, primitive_value_t value)
    {
        return std::make_unique<LiteralTypeRef>(std::move(token), std::move(value));
    }

    void accept(TypeRefVisitor<void>& visitor) override
    {
        return visitor.visit_literal_type(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return token;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return primitive_to_string(value);
    }
};