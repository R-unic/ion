#pragma once
#include <algorithm>
#include <utility>

#include "ion/token.h"
#include "ion/ast/node.h"

class InstanceTagDeclarator final : public InstanceDeclarator
{
public:
    Token hashtag_token;

    explicit InstanceTagDeclarator(Token hashtag_token, Token name)
        : InstanceDeclarator(std::move(name)),
          hashtag_token(std::move(hashtag_token))
    {
    }

    static statement_ptr_t create(Token hashtag_token, Token name)
    {
        return std::make_unique<InstanceTagDeclarator>(std::move(hashtag_token), std::move(name));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_instance_tag_declarator(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return hashtag_token;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return name;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return '#' + name.get_text();
    }
};