#pragma once
#include <algorithm>
#include <utility>
#include <vector>

#include "utility.h"
#include "ast/node.h"

class Block final : public Statement
{
public:
    Token l_brace, r_brace;
    std::vector<statement_ptr_t>* statements;
    
    explicit Block(Token l_brace, Token r_brace, std::vector<statement_ptr_t>* statements)
        : l_brace(std::move(l_brace)),
          r_brace(std::move(r_brace)),
          statements(statements)    
    {
    }
    
    static statement_ptr_t create(Token l_brace, Token r_brace, std::vector<statement_ptr_t>* statements)
    {
        return std::make_unique<Block>(std::move(l_brace), std::move(r_brace), statements);
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_block(*this);
    }

    [[nodiscard]] FileSpan get_span() const override
    {
        return create_span(statements->front()->get_span().start, statements->back()->get_span().end);
    }
    
    [[nodiscard]] std::string get_text() const override
    {
        return "{\n\t" + join_by(*statements, "\n\t") + "\n}";
    }
};