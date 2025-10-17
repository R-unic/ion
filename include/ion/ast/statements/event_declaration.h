#pragma once
#include <algorithm>
#include <optional>
#include <vector>

#include "ion/token.h"
#include "ion/ast/node.h"

class EventDeclaration final : public NamedDeclaration
{
public:
    Token event_keyword;
    std::optional<Token> l_paren, r_paren;
    /** Generics */
    std::optional<TypeListClause*> type_parameters;
    /** Types of parameters passed when firing the event */
    std::vector<type_ref_ptr_t> parameter_types;

    explicit EventDeclaration(
        Token event_keyword,
        Token name,
        const std::optional<TypeListClause*> type_parameters,
        std::optional<Token> l_paren,
        std::vector<type_ref_ptr_t> parameter_types,
        std::optional<Token> r_paren)
        : NamedDeclaration(std::move(name)),
          event_keyword(std::move(event_keyword)),
          l_paren(std::move(l_paren)),
          r_paren(std::move(r_paren)),
          type_parameters(type_parameters),
          parameter_types(std::move(parameter_types))
    {
    }

    ~EventDeclaration() override
    {
        if (type_parameters.has_value())
            delete *type_parameters;
    }

    static statement_ptr_t create(
        Token event_keyword,
        Token name,
        const std::optional<TypeListClause*> type_parameters,
        std::optional<Token> l_paren,
        std::vector<type_ref_ptr_t> parameter_types,
        std::optional<Token> r_paren)
    {
        return std::make_unique<EventDeclaration>(std::move(event_keyword), std::move(name), type_parameters,
                                                  std::move(l_paren), std::move(parameter_types), std::move(r_paren));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_event_declaration(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return event_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return r_paren.has_value()
                   ? *r_paren
                   : type_parameters.has_value()
                         ? type_parameters.value()->get_last_token()
                         : name;
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto type_parameters_text = type_parameters.has_value() ? type_parameters.value()->get_text() : "";
        const auto l_paren_text = l_paren.has_value() ? "(" : "";
        const auto parameter_types_text = join_by(parameter_types, ", ");
        const auto r_paren_text = r_paren.has_value() ? ")" : "";

        return "event " + name.get_text() + type_parameters_text
               + l_paren_text + parameter_types_text + r_paren_text;
    }
};