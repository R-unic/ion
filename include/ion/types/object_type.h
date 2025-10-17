#pragma once
#include <string>

#include "literal_type.h"
#include "type.h"

struct ObjectType : Type
{
    struct TypePtrHash
    {
        size_t operator()(const std::shared_ptr<Type>& ptr) const noexcept
        {
            return std::hash<const Type*> {}(ptr.get());
        }
    };

    struct TypePtrEq
    {
        bool operator()(const std::shared_ptr<Type>& a, const std::shared_ptr<Type>& b) const noexcept
        {
            return a.get() == b.get();
        }
    };

    using member_map_t = std::unordered_map<type_ptr_t, type_ptr_t, TypePtrHash, TypePtrEq>;
    member_map_t members;

    explicit ObjectType(member_map_t members)
        : members(std::move(members))
    {
    }

    [[nodiscard]] type_ptr_t as_shared() const override
    {
        return std::make_shared<ObjectType>(*this);
    }

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