#pragma once
#include <vector>

#include "ion/token.h"
#include "../../utility/basic.h"
#include "ion/ast/node.h"

class UnionTypeRef final : public TypeRef
{
public:
    std::vector<Token> pipe_tokens;
    std::vector<type_ref_ptr_t> types;

    explicit UnionTypeRef(std::vector<Token> pipe_tokens, std::vector<type_ref_ptr_t> types)
        : pipe_tokens(std::move(pipe_tokens)),
          types(std::move(types))
    {
    }

    static type_ref_ptr_t create(std::vector<Token> pipe_tokens, std::vector<type_ref_ptr_t> types)
    {
        return std::make_unique<UnionTypeRef>(std::move(pipe_tokens), std::move(types));
    }

    void accept(TypeRefVisitor<void>& visitor) override
    {
        return visitor.visit_union_type(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return pipe_tokens.front().span.start < types.front()->get_span().start
                   ? pipe_tokens.front()
                   : types.front()->get_first_token();
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return types.back()->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        return join_by(types, " | ");
    }
};