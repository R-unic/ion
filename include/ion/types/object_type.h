#pragma once
#include <string>

#include "literal_type.h"
#include "type.h"

struct ObjectType : Type
{
    using member_map_t = std::unordered_map<type_ptr_t, type_ptr_t, TypePtrHash, TypePtrEq>;
    member_map_t members;

    explicit ObjectType(member_map_t members)
        : members(std::move(members))
    {
    }

    [[nodiscard]] bool is_same(const type_ptr_t& other) const override
    {
        CAST_CHECK(object, ObjectType);
        auto is_same = true;
        for (const auto& [key, value] : members)
            is_same = is_same && object->members.contains(key) && object->members.at(key)->is_same(value);

        return is_same;
    }

    TYPE_OVERRIDES(object, ObjectType);

    [[nodiscard]] std::string to_string() const override
    {
        std::ostringstream oss;
        oss << "{ ";

        auto first = true;
        for (const auto& [key_type, value_type] : members)
        {
            if (!first)
                oss << ", ";

            first = false;
            auto key_text = '[' + key_type->to_string() + ']';
            if (key_type->is_literal())
                if (const auto type = std::static_pointer_cast<LiteralType>(key_type); type->kind == PrimitiveTypeKind::String)
                    key_text = type->to_string().substr(1, type->to_string().length() - 2);

            oss << key_text << ": " << value_type->to_string();
        }

        oss << " }";
        return oss.str();
    }
};