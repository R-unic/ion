#pragma once
#include <algorithm>

#include "ion/token.h"
#include "ion/ast/node.h"

class InterfaceMethod final : public NamedDeclaration
{
public:
    Token fn_keyword, l_paren, r_paren, colon_token;
    /** Generics */
    std::optional<TypeListClause*> type_parameters;
    /** Types of parameters passed when calling the method */
    std::vector<type_ref_ptr_t> parameter_types;
    type_ref_ptr_t return_type;

    explicit InterfaceMethod(Token fn_keyword, Token name, std::optional<TypeListClause*> type_parameters, Token l_paren,
                             std::vector<type_ref_ptr_t> parameter_types, Token r_paren, Token colon_token, type_ref_ptr_t return_type)
        : NamedDeclaration(std::move(name)),
          fn_keyword(std::move(fn_keyword)),
          l_paren(std::move(l_paren)),
          r_paren(std::move(r_paren)),
          colon_token(std::move(colon_token)),
          type_parameters(type_parameters),
          parameter_types(std::move(parameter_types)),
          return_type(std::move(return_type))
    {
    }

    static statement_ptr_t create(Token fn_keyword, Token name, std::optional<TypeListClause*> type_parameters, Token l_paren,
                                  std::vector<type_ref_ptr_t> parameter_types, Token r_paren, Token colon_token, type_ref_ptr_t return_type)
    {
        return std::make_unique<InterfaceMethod>(std::move(fn_keyword), std::move(name), type_parameters,
                                                 std::move(l_paren), std::move(parameter_types), std::move(r_paren),
                                                 std::move(colon_token), std::move(return_type));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_interface_method(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return fn_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return return_type->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto type_parameters_text = type_parameters.has_value() ? type_parameters.value()->get_text() : "";
        const auto parameter_types_text = '(' + join_by(parameter_types, ", ") + ')';
        const auto return_type_text = ": " + return_type->get_text();

        return "fn " + name.get_text() + type_parameters_text + parameter_types_text + return_type_text;
    }
};