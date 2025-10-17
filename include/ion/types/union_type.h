#pragma once
#include <string>
#include <vector>

#include "type.h"

struct UnionType final : Type
{
    std::vector<type_ptr_t> types;

    explicit UnionType(std::vector<type_ptr_t> types)
        : types(std::move(types))
    {
    }

    [[nodiscard]] type_ptr_t as_shared() const override
    {
        return std::make_shared<UnionType>(*this);
    }

    [[nodiscard]] std::string to_string() const override
    {
        return join_types_by(types, " | ");
    }
};