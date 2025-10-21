#pragma once
#include <string>
#include <vector>

#include "type.h"

struct IntersectionType final : Type
{
    std::vector<type_ptr_t> types;

    explicit IntersectionType(std::vector<type_ptr_t> types)
        : types(std::move(types))
    {
    }

    [[nodiscard]] bool is_same(const type_ptr_t& other) const override
    {
        CAST_CHECK(intersection, IntersectionType);
        return is_list_same(types, intersection->types);
    }

    TYPE_OVERRIDES(intersection, IntersectionType);

    [[nodiscard]] std::string to_string() const override
    {
        return join_by(types, " & ");
    }
};