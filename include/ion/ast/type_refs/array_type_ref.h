#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"

class ArrayTypeRef final : public TypeRef
{
public:
    type_ref_ptr_t element_type;
    Token l_bracket, r_bracket;

    explicit ArrayTypeRef(type_ref_ptr_t element_type, Token l_bracket, Token r_bracket)
        : element_type(std::move(element_type)),
          l_bracket(std::move(l_bracket)),
          r_bracket(std::move(r_bracket))
    {
    }

    static type_ref_ptr_t create(type_ref_ptr_t element_type, Token l_bracket, Token r_bracket)
    {
        return std::make_unique<ArrayTypeRef>(std::move(element_type), std::move(l_bracket), std::move(r_bracket));
    }

    void accept(TypeRefVisitor<void>& visitor) override
    {
        return visitor.visit_array_type(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return element_type->get_first_token();
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return r_bracket;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return element_type->get_text() + "[]";
    }
};