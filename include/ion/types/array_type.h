#pragma once
#include <string>
#include <vector>

#include "type.h"

struct ArrayType final : Type
{
    type_ptr_t element_type;

    explicit ArrayType(type_ptr_t element_type)
        : element_type(std::move(element_type))
    {
    }

    [[nodiscard]] bool is_literal_array() const override
    {
        return element_type->is_literal_like();
    }

    [[nodiscard]] bool is_same(const type_ptr_t& other) const override
    {
        CAST_CHECK(array, ArrayType);
        return element_type->is_same(array->element_type);
    }

    TYPE_OVERRIDES(array, ArrayType);

    [[nodiscard]] std::string to_string() const override
    {
        return element_type->to_string() + "[]";
    }
};