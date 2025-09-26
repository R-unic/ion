#pragma once
#include "token.h"
#include "ast/node.h"

class PrimitiveType final : public TypeRef
{
public:
    Token keyword;

    explicit PrimitiveType(Token keyword)
        : keyword(std::move(keyword))
    {
    }
    
    static type_ref_ptr_t create(Token keyword)
    {
        return std::make_unique<PrimitiveType>(std::move(keyword));
    }
    
    void accept(TypeRefVisitor<void>& visitor) override
    {
        return visitor.visit_primitive_type(*this);
    }

    [[nodiscard]] FileSpan get_span() const override
    {
        return keyword.span;
    }
    
    [[nodiscard]] std::string get_text() const override
    {
        return keyword.get_text();
    }
};
