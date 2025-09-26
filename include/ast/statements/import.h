#pragma once
#include <algorithm>
#include <optional>

#include "token.h"
#include "utility.h"
#include "ast/node.h"

class Import final : public Statement
{
public:
    Token import_keyword;
    std::vector<Token> names;
    std::optional<Token> from_keyword;
    Token module_name;
    
    explicit Import(Token import_keyword, std::vector<Token> names, std::optional<Token> from_keyword, Token module_name)
        : import_keyword(std::move(import_keyword)),
        names(std::move(names)),
        from_keyword(std::move(from_keyword)),
        module_name(std::move(module_name))
    {
    }
    
    static statement_ptr_t create(Token import_keyword, std::vector<Token> names, std::optional<Token> from_keyword, Token module_name)
    {
        return std::make_unique<Import>(std::move(import_keyword), std::move(names), std::move(from_keyword), std::move(module_name));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_import(*this);
    }

    [[nodiscard]] FileSpan get_span() const override
    {
        return create_span(import_keyword.span.start, module_name.span.end);
    }
    
    [[nodiscard]] std::string get_text() const override
    {
        return "import " + join_tokens_by(names, ", ") + " from " + module_name.get_text();
    }
};