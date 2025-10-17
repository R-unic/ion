#pragma once
#include "ion/token.h"
#include "ion/ast/node.h"

class NullableTypeRef final : public TypeRef
{
public:
    type_ref_ptr_t non_nullable_type;
    Token question_token;

    explicit NullableTypeRef(type_ref_ptr_t non_nullable_type, Token question_token)
        : non_nullable_type(std::move(non_nullable_type)),
          question_token(std::move(question_token))
    {
    }

    static type_ref_ptr_t create(type_ref_ptr_t non_nullable_type, Token question_token)
    {
        return std::make_unique<NullableTypeRef>(std::move(non_nullable_type), std::move(question_token));
    }

    void accept(TypeRefVisitor<void>& visitor) override
    {
        return visitor.visit_nullable_type(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return non_nullable_type->get_first_token();
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return question_token;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return non_nullable_type->get_text() + '?';
    }
};