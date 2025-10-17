#pragma once
#include <algorithm>

#include "ion/token.h"
#include "ion/ast/node.h"

class Export final : public Statement
{
public:
    Token export_keyword;
    statement_ptr_t statement;

    explicit Export(Token export_keyword, statement_ptr_t statement)
        : export_keyword(std::move(export_keyword)),
          statement(std::move(statement))
    {
    }

    static statement_ptr_t create(Token export_keyword, statement_ptr_t statement)
    {
        return std::make_unique<Export>(std::move(export_keyword), std::move(statement));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_export(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return export_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return statement->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        return "export " + statement->get_text();
    }
};