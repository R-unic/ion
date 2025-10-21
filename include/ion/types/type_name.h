#pragma once
#include <string>

#include "type.h"
#include "ion/utility/ast.h"

struct TypeName final : Type
{
    std::string name;
    std::vector<type_ptr_t> type_arguments;

    explicit TypeName(std::string name, std::vector<type_ptr_t> type_arguments)
        : name(std::move(name)),
          type_arguments(std::move(type_arguments))
    {
    }

    [[nodiscard]] bool is_same(const type_ptr_t& other) const override
    {
        // TODO: check type params
        CAST_CHECK(type_name, TypeName);
        return name == type_name->name;
    }

    TYPE_OVERRIDES(type_name, TypeName);

    [[nodiscard]] std::string to_string() const override
    {
        return name + generics_to_string(type_arguments);
    }
};