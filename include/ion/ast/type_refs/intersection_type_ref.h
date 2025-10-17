#pragma once
#include <vector>

#include "ion/token.h"
#include "../../utility/basic.h"
#include "ion/ast/node.h"

class IntersectionTypeRef final : public TypeRef
{
public:
    std::vector<Token> ampersand_tokens;
    std::vector<type_ref_ptr_t> types;

    explicit IntersectionTypeRef(std::vector<Token> ampersand_tokens, std::vector<type_ref_ptr_t> types)
        : ampersand_tokens(std::move(ampersand_tokens)),
          types(std::move(types))
    {
    }

    static type_ref_ptr_t create(std::vector<Token> ampersand_tokens, std::vector<type_ref_ptr_t> types)
    {
        return std::make_unique<IntersectionTypeRef>(std::move(ampersand_tokens), std::move(types));
    }

    void accept(TypeRefVisitor<void>& visitor) override
    {
        return visitor.visit_intersection_type(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return ampersand_tokens.front().span.start < types.front()->get_span().start
                   ? ampersand_tokens.front()
                   : types.front()->get_first_token();
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return types.back()->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        return join_by(types, " & ");
    }
};