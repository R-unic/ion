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

    [[nodiscard]] type_ptr_t as_shared() const override
    {
        return std::make_shared<IntersectionType>(*this);
    }

    [[nodiscard]] std::string to_string() const override
    {
        return join_types_by(types, " & ");
    }
};