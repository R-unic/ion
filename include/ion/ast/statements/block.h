#pragma once
#include "../containers/braced_statement_list.h"
#include "ion/ast/node.h"

class Block final : public Statement
{
public:
    BracedStatementList* braced_statement_list;

    explicit Block(BracedStatementList* braced_statement_list)
        : braced_statement_list(braced_statement_list)
    {
    }

    ~Block() override
    {
        delete braced_statement_list;
    }

    static statement_ptr_t create(BracedStatementList* braced_statement_list)
    {
        return std::make_unique<Block>(braced_statement_list);
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_block(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return braced_statement_list->get_first_token();
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return braced_statement_list->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        return braced_statement_list->get_text();
    };

    [[nodiscard]] bool is_block() const override
    {
        return true;
    }
};