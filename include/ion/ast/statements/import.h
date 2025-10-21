#pragma once
#include <algorithm>
#include <optional>

#include "ion/token.h"
#include "../../utility/basic.h"
#include "ion/ast/node.h"

class Import final : public Statement
{
public:
    Token import_keyword;
    std::vector<Token> names;
    std::optional<Token> from_keyword;
    Token module_name;

    explicit Import(Token import_keyword, std::vector<Token> names, std::optional<Token> from_keyword,
                    Token module_name)
        : import_keyword(std::move(import_keyword)),
          names(std::move(names)),
          from_keyword(std::move(from_keyword)),
          module_name(std::move(module_name))
    {
    }

    static statement_ptr_t create(Token import_keyword, std::vector<Token> names, std::optional<Token> from_keyword,
                                  Token module_name)
    {
        return std::make_unique<Import>(std::move(import_keyword), std::move(names), std::move(from_keyword),
                                        std::move(module_name));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_import(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return import_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return module_name;
    }

    [[nodiscard]] std::string get_text() const override
    {
        return "import " + join_by(names, ", ") + " from " + module_name.get_text();
    }
};