#pragma once
#include <sstream>
#include <ranges>
#include <vector>

#include "../containers/type_list_clause.h"
#include "ion/token.h"
#include "ion/ast/node.h"

class Decorator final : public Statement
{
public:
    Token at_token, name;
    std::optional<Token> l_paren, r_paren;
    std::vector<expression_ptr_t> arguments;

    explicit Decorator(Token at_token, Token name, std::optional<Token> l_paren, std::optional<Token> r_paren,
                       std::vector<expression_ptr_t> arguments)
        : at_token(std::move(at_token)),
          name(std::move(name)),
          l_paren(std::move(l_paren)),
          r_paren(std::move(r_paren)),
          arguments(std::move(arguments))
    {
    }

    static statement_ptr_t create(Token at_token, Token name, std::optional<Token> l_paren, std::optional<Token> r_paren,
                                  std::vector<expression_ptr_t> arguments)
    {
        return std::make_unique<Decorator>(std::move(at_token), std::move(name), std::move(l_paren), std::move(r_paren),
                                           std::move(arguments));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_decorator(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return at_token;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return r_paren.value_or(name);
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto arguments_text = join_by(arguments, ", ");
        return '@' + name.get_text() + (!arguments.empty() ? '(' + arguments_text + ')' : "");
    }
};