#pragma once
#include <vector>

#include "../containers/type_list_clause.h"
#include "ion/token.h"
#include "ion/ast/node.h"

class FunctionTypeRef final : public TypeRef
{
public:
    /** Generics */
    std::optional<TypeListClause*> type_parameters;
    Token l_paren, r_paren, long_arrow;
    /** Types of parameters passed when calling the function */
    std::vector<type_ref_ptr_t> parameter_types;
    type_ref_ptr_t return_type;

    explicit FunctionTypeRef(const std::optional<TypeListClause*> type_parameters,
                             Token l_paren, std::vector<type_ref_ptr_t> parameter_types, Token r_paren,
                             Token long_arrow, type_ref_ptr_t return_type)
        : type_parameters(type_parameters),
          l_paren(std::move(l_paren)),
          r_paren(std::move(r_paren)),
          long_arrow(std::move(long_arrow)),
          parameter_types(std::move(parameter_types)),
          return_type(std::move(return_type))
    {
    }

    ~FunctionTypeRef() override
    {
        if (type_parameters.has_value())
            delete *type_parameters;
    }

    static type_ref_ptr_t create(const std::optional<TypeListClause*> type_parameters,
                                 Token l_paren, std::vector<type_ref_ptr_t> parameter_types, Token r_paren,
                                 Token long_arrow, type_ref_ptr_t return_type)
    {
        return std::make_unique<FunctionTypeRef>(type_parameters, std::move(l_paren), std::move(parameter_types), std::move(r_paren),
                                                 std::move(long_arrow), std::move(return_type));
    }

    void accept(TypeRefVisitor<void>& visitor) override
    {
        return visitor.visit_function_type(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return type_parameters.has_value() ? type_parameters.value()->get_first_token() : l_paren;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return return_type->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto type_parameters_text = type_parameters.has_value() ? type_parameters.value()->get_text() : "";
        return type_parameters_text + '(' + join_by(parameter_types, ", ") + ") -> " + return_type->get_text();
    }
};