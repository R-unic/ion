#pragma once
#include <algorithm>

#include "ion/token.h"
#include "ion/ast/node.h"

class InstanceNameDeclarator final : public InstanceDeclarator
{
public:
    explicit InstanceNameDeclarator(Token name_literal) : InstanceDeclarator(std::move(name_literal))
    {
    }

    static statement_ptr_t create(Token name_literal)
    {
        return std::make_unique<InstanceNameDeclarator>(std::move(name_literal));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_instance_name_declarator(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return name;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return name.get_text();
    }
};