#pragma once
#include <algorithm>
#include <optional>

#include "token.h"
#include "ast/node.h"

class While final : public Statement
{
public:
    Token while_keyword;
    expression_ptr_t condition;
    statement_ptr_t body;
    
    explicit While(Token while_keyword, expression_ptr_t condition, statement_ptr_t body)
        : while_keyword(std::move(while_keyword)),
        condition(std::move(condition)),
        body(std::move(body))
    {
    }
    
    static statement_ptr_t create(Token while_keyword, expression_ptr_t condition, statement_ptr_t body)
    {
        return std::make_unique<While>(std::move(while_keyword), std::move(condition), std::move(body));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_while(*this);
    }
};