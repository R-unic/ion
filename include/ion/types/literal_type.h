#pragma once
#include <string>

#include "primitive_type.h"
#include "type.h"

struct LiteralType final : PrimitiveType
{
    primitive_value_t value;

    explicit LiteralType(primitive_value_t value)
        : PrimitiveType(PrimitiveTypeKind::Void),
          value(std::move(value))
    {
        this->kind = std::visit([]<typename T>(const T&)
        {
            using type_t = std::decay_t<T>;
            if constexpr (std::is_same_v<type_t, std::string>)
                return PrimitiveTypeKind::String;
            if constexpr (std::is_same_v<type_t, double>)
                return PrimitiveTypeKind::Number;
            if constexpr (std::is_same_v<type_t, bool>)
                return PrimitiveTypeKind::Bool;
        }, value);
    }

    [[nodiscard]] bool is_literal() const override
    {
        return true;
    }

    [[nodiscard]] type_ptr_t as_primitive() const
    {
        return std::make_shared<PrimitiveType>(*this);
    }

    [[nodiscard]] type_ptr_t as_shared() const override
    {
        return std::make_shared<LiteralType>(*this);
    }

    [[nodiscard]] std::string to_string() const override
    {
        return primitive_to_string(value);
    }
};