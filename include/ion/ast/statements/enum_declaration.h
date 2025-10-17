#pragma once
#include <algorithm>

#include "ion/token.h"
#include "ion/ast/node.h"

class EnumDeclaration final : public NamedDeclaration
{
public:
    Token enum_keyword;
    BracedStatementList* members;

    explicit EnumDeclaration(Token enum_keyword, Token name, BracedStatementList* members)
        : NamedDeclaration(std::move(name)),
          enum_keyword(std::move(enum_keyword)),
          members(members)
    {
    }

    ~EnumDeclaration() override
    {
        delete members;
    }

    static statement_ptr_t create(Token enum_keyword, Token name, BracedStatementList* members)
    {
        return std::make_unique<EnumDeclaration>(std::move(enum_keyword), std::move(name), members);
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_enum_declaration(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return enum_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return members->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        return "enum " + name.get_text() + ' ' + members->get_text();
    }
};