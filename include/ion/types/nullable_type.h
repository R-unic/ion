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

    [[nodiscard]] type_ptr_t as_shared() const override
    {
        return std::make_shared<NullableType>(*this);
    }

    [[nodiscard]] std::string to_string() const override
    {
        return non_nullable_type->to_string() + '?';
    }
};