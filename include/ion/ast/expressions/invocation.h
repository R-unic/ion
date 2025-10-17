#pragma once
#include <sstream>
#include <ranges>
#include <vector>

#include "../containers/type_list_clause.h"
#include "ion/token.h"
#include "ion/ast/node.h"

class Invocation : public Expression
{
public:
    Token l_paren, r_paren;
    expression_ptr_t callee;
    std::optional<Token> bang_token;
    std::optional<TypeListClause*> type_arguments;
    std::vector<expression_ptr_t> arguments;

    explicit Invocation(Token l_paren, Token r_paren, expression_ptr_t callee, std::optional<Token> bang_token,
                        const std::optional<TypeListClause*> type_arguments, std::vector<expression_ptr_t> arguments)
        : l_paren(std::move(l_paren)),
          r_paren(std::move(r_paren)),
          callee(std::move(callee)),
          bang_token(std::move(bang_token)),
          type_arguments(type_arguments),
          arguments(std::move(arguments))
    {
    }

    ~Invocation() override
    {
        if (type_arguments.has_value())
            delete *type_arguments;
    }

    static expression_ptr_t create(Token l_paren, Token r_paren, expression_ptr_t callee, std::optional<Token> bang_token,
                                   const std::optional<TypeListClause*> type_arguments, std::vector<expression_ptr_t> arguments)
    {
        return std::make_unique<Invocation>(std::move(l_paren), std::move(r_paren), std::move(callee), std::move(bang_token),
                                            type_arguments, std::move(arguments));
    }

    void accept(ExpressionVisitor<void>& visitor) override
    {
        return visitor.visit_invocation(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return callee->get_first_token();
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return r_paren;
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto arguments_text = join_by(arguments, ", ");
        const auto type_arguments_text = type_arguments.has_value() ? type_arguments.value()->get_text() : "";
        const auto bang_text = bang_token.has_value() ? "!" : "";
        return callee->get_text() + bang_text + type_arguments_text + '(' + arguments_text + ')';
    }
};