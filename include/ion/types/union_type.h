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

    [[nodiscard]] bool is_literal_union() const override
    {
        auto is_literal = true;
        for (const auto& type : types)
            is_literal = is_literal && type->is_literal_like();

        return is_literal;
    }

    [[nodiscard]] bool is_same(const type_ptr_t& other) const override
    {
        if (!other->is_union())
            return false;

        const auto union_ = std::dynamic_pointer_cast<UnionType>(other);
        return is_list_same(types, union_->types);
    }

    TYPE_OVERRIDES(union, UnionType);

    [[nodiscard]] std::string to_string() const override
    {
        return join_by(types, " | ");
    }
};