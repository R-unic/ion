#pragma once
#include <string>
#include <vector>

#include "type.h"

struct FunctionType final : Type
{
    std::vector<type_ptr_t> type_parameters, parameters;
    type_ptr_t return_type;

    explicit FunctionType(std::vector<type_ptr_t> type_parameters, std::vector<type_ptr_t> parameters, type_ptr_t return_type)
        : type_parameters(std::move(type_parameters)),
          parameters(std::move(parameters)),
          return_type(std::move(return_type))
    {
    }

    [[nodiscard]] bool is_same(const type_ptr_t& other) const override
    {
        CAST_CHECK(function, FunctionType);
        return is_list_same(type_parameters, function->type_parameters)
               && is_list_same(parameters, function->parameters)
               && return_type->is_same(function->return_type);
    }

    TYPE_OVERRIDES(function, FunctionType);

    [[nodiscard]] std::string to_string() const override
    {
        const auto type_parameters_text = type_parameters.empty() ? "" : '<' + join_by(type_parameters, ", ") + '>';
        const auto parameters_text = join_by(parameters, ", ");
        return type_parameters_text + '(' + parameters_text + ") -> " + return_type->to_string();
    }
};