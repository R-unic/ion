#pragma once
#include "token.h"
#include "ast/node.h"

class NullableType final : public TypeRef
{
public:
    type_ref_ptr_t non_nullable_type;
    Token question_token;

    explicit NullableType(type_ref_ptr_t non_nullable_type, Token question_token)
        : non_nullable_type(std::move(non_nullable_type)),
        question_token(std::move(question_token))
    {
    }
    
    static type_ref_ptr_t create(type_ref_ptr_t non_nullable_type, Token question_token)
    {
        return std::make_unique<NullableType>(std::move(non_nullable_type), std::move(question_token));
    }
    
    void accept(TypeRefVisitor<void>& visitor) override
    {
        return visitor.visit_nullable_type(*this);
    }

    [[nodiscard]] FileSpan get_span() const override
    {
        return create_span(non_nullable_type->get_span().start, question_token.span.end);
    }
    
    [[nodiscard]] std::string get_text() const override
    {
        return non_nullable_type->get_text() + '?';
    }
};
