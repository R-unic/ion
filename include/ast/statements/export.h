#pragma once
#include <algorithm>
#include <optional>

#include "token.h"
#include "ast/node.h"

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

    [[nodiscard]] FileSpan get_span() const override
    {
        return create_span(export_keyword.span.start, statement->get_span().end);
    }
    
    [[nodiscard]] std::string get_text() const override
    {
        return "export " + statement->get_text();
    }
};