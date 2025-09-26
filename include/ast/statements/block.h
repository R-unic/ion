#pragma once
#include <algorithm>
#include <vector>

#include "ast/node.h"

class Block final : public Statement
{
public:
    std::vector<statement_ptr_t>* statements;
    
    explicit Block(std::vector<statement_ptr_t>* statements)
        : statements(statements)    
    {
    }
    
    static statement_ptr_t create(std::vector<statement_ptr_t>* statements)
    {
        return std::make_unique<Block>(statements);
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_block(*this);
    }
};