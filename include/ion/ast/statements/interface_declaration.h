#pragma once
#include <algorithm>

#include "named_declaration.h"
#include "ion/token.h"
#include "ion/ast/node.h"

class InterfaceDeclaration final : public NamedDeclaration
{
public:
    Token interface_keyword;
    std::optional<TypeListClause*> type_parameters;
    BracedStatementList* members;

    explicit InterfaceDeclaration(Token interface_keyword, Token name, const std::optional<TypeListClause*> type_parameters,
                                  BracedStatementList* members)
        : NamedDeclaration(std::move(name)),
          interface_keyword(std::move(interface_keyword)),
          type_parameters(type_parameters),
          members(members)
    {
    }

    ~InterfaceDeclaration() override
    {
        if (type_parameters.has_value())
            delete *type_parameters;

        delete members;
    }

    static statement_ptr_t create(Token interface_keyword, Token name, const std::optional<TypeListClause*> type_parameters, BracedStatementList* members)
    {
        return std::make_unique<InterfaceDeclaration>(std::move(interface_keyword), std::move(name), type_parameters, members);
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