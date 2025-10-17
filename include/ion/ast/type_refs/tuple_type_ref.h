#pragma once
#include <vector>

#include "ion/token.h"
#include "ion/ast/node.h"

class TupleTypeRef final : public TypeRef
{
public:
    Token l_paren, r_paren;
    std::vector<type_ref_ptr_t> element_types;

    explicit TupleTypeRef(Token l_paren, std::vector<type_ref_ptr_t> element_types, Token r_paren)
        : l_paren(std::move(l_paren)),
          r_paren(std::move(r_paren)),
          element_types(std::move(element_types))
    {
    }

    static type_ref_ptr_t create(Token l_paren, std::vector<type_ref_ptr_t> elements, Token r_paren)
    {
        return std::make_unique<TupleTypeRef>(std::move(l_paren), std::move(elements), std::move(r_paren));
    }

    void accept(TypeRefVisitor<void>& visitor) override
    {
        return visitor.visit_tuple_type(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return l_paren;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return r_paren;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return '(' + join_by(element_types, ", ") + ')';
    }
};