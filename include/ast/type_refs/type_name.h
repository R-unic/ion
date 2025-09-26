#pragma once
#include "token.h"
#include "ast/node.h"

class TypeName final : public TypeRef
{
public:
    Token name;

    explicit TypeName(Token name)
        : name(std::move(name))
    {
    }
    
    static type_ref_ptr_t create(Token name)
    {
        return std::make_unique<TypeName>(std::move(name));
    }
    
    void accept(TypeRefVisitor<void>& visitor) override
    {
        return visitor.visit_type_name(*this);
    }

    [[nodiscard]] FileSpan get_span() const override
    {
        return name.span;
    }
    
    [[nodiscard]] std::string get_text() const override
    {
        return name.get_text();
    }
};
