#pragma once
#include <algorithm>
#include <optional>

#include "ion/token.h"
#include "ion/ast/node.h"

class If final : public Statement
{
public:
    Token if_keyword;
    expression_ptr_t condition;
    statement_ptr_t then_branch;
    std::optional<Token> else_keyword;
    std::optional<statement_ptr_t> else_branch;

    explicit If(Token if_keyword, expression_ptr_t condition, statement_ptr_t then_branch,
                std::optional<Token> else_keyword, std::optional<statement_ptr_t> else_branch)
        : if_keyword(std::move(if_keyword)),
          condition(std::move(condition)),
          then_branch(std::move(then_branch)),
          else_keyword(std::move(else_keyword)),
          else_branch(std::move(else_branch))
    {
    }

    static statement_ptr_t create(Token if_keyword, expression_ptr_t condition, statement_ptr_t then_branch,
                                  std::optional<Token> else_keyword, std::optional<statement_ptr_t> else_branch)
    {
        return std::make_unique<If>(std::move(if_keyword), std::move(condition), std::move(then_branch),
                                    std::move(else_keyword), std::move(else_branch));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_if(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return if_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return else_branch.has_value() ? else_branch.value()->get_last_token() : then_branch->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto separator = then_branch->is_block() ? ' ' : '\n';
        const auto else_text = else_branch.has_value()
                                   ? std::string("else ") + separator + else_branch.value()->get_text()
                                   : "";

        return "if " + condition->get_text() + separator + then_branch->get_text() + else_text;
    }
};