#pragma once
#include <string>
#include <vector>

#include "type.h"

struct NullableType final : Type
{
    type_ptr_t non_nullable_type;

    explicit NullableType(type_ptr_t non_nullable_type)
        : non_nullable_type(std::move(non_nullable_type))
    {
    }

    [[nodiscard]] bool is_same(const type_ptr_t& other) const override
    {
        CAST_CHECK(nullable, NullableType);
        return non_nullable_type->is_same(nullable->non_nullable_type);
    }

    TYPE_OVERRIDES(nullable, NullableType);

    [[nodiscard]] std::string to_string() const override
    {
        return non_nullable_type->to_string() + '?';
    }
};