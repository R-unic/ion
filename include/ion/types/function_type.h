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

    [[nodiscard]] type_ptr_t as_shared() const override
    {
        return std::make_shared<FunctionType>(*this);
    }

    [[nodiscard]] std::string to_string() const override
    {
        const auto type_parameters_text = type_parameters.empty() ? "" : '<' + join_types_by(type_parameters, ", ") + '>';
        const auto parameters_text = join_types_by(parameters, ", ");
        return type_parameters_text + '(' + parameters_text + ") -> " + return_type->to_string();
    }
};