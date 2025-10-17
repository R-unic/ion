#pragma once
#include <algorithm>

#include "named_declaration.h"
#include "ion/token.h"
#include "ion/ast/node.h"

class InterfaceDeclaration final : public NamedDeclaration
{
public:
    Token interface_keyword;
    BracedStatementList* members;

    explicit InterfaceDeclaration(Token interface_keyword, Token name, BracedStatementList* members)
        : NamedDeclaration(name),
          interface_keyword(std::move(interface_keyword)),
          members(members)
    {
    }

    ~InterfaceDeclaration() override
    {
        delete members;
    }

    static statement_ptr_t create(Token interface_keyword, Token name, BracedStatementList* members)
    {
        return std::make_unique<InterfaceDeclaration>(std::move(interface_keyword), std::move(name), members);
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_interface_declaration(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return interface_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return members->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        return "interface " + name.get_text() + ' ' + members->get_text();
    }
};