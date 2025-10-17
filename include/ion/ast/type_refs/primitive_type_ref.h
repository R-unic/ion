#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"

class PrimitiveTypeRef final : public TypeRef
{
public:
    Token keyword;

    explicit PrimitiveTypeRef(Token keyword)
        : keyword(std::move(keyword))
    {
    }

    static type_ref_ptr_t create(Token keyword)
    {
        return std::make_unique<PrimitiveTypeRef>(std::move(keyword));
    }

    void accept(TypeRefVisitor<void>& visitor) override
    {
        return visitor.visit_primitive_type(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return keyword;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return keyword.get_text();
    }
};