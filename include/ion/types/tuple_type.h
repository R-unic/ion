#pragma once
#include <string>
#include <vector>

#include "type.h"

struct TupleType final : Type
{
    std::vector<type_ptr_t> element_types;

    explicit TupleType(std::vector<type_ptr_t> element_types)
        : element_types(std::move(element_types))
    {
    }

    [[nodiscard]] bool is_literal_tuple() const override
    {
        auto is_literal = true;
        for (const auto& type : element_types)
            is_literal = is_literal && type->is_literal_like();

        return is_literal;
    }

    [[nodiscard]] bool is_same(const type_ptr_t& other) const override
    {
        CAST_CHECK(tuple, TupleType);
        return is_list_same(element_types, tuple->element_types);
    }

    TYPE_OVERRIDES(tuple, TupleType);

    [[nodiscard]] std::string to_string() const override
    {
        return '(' + join_types_by(element_types, ", ") + ')';
    }
};