#pragma once
#include <algorithm>
#include <optional>

#include "token.h"
#include "ast/node.h"

class VariableDeclaration final : public Statement
{
public:
    // TODO: types
    Token let_keyword, name;
    std::optional<Token> equals_token;
    std::optional<expression_ptr_t> initializer;
    
    explicit VariableDeclaration(Token let_keyword, Token name, std::optional<Token> equals_token, std::optional<expression_ptr_t> initializer)
        : let_keyword(std::move(let_keyword)),
        name(std::move(name)),
        equals_token(std::move(equals_token)),
        initializer(std::move(initializer))
    {
    }
    
    static statement_ptr_t create(Token let_keyword, Token name, std::optional<Token> equals_token, std::optional<expression_ptr_t> initializer)
    {
        return std::make_unique<VariableDeclaration>(std::move(let_keyword), std::move(name), std::move(equals_token), std::move(initializer));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_variable_declaration(*this);
    }
};