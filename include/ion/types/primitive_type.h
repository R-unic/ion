#pragma once
#include <string>

#include "type.h"

enum class PrimitiveTypeKind
{
    Number,
    String,
    Bool,
    Void
};

struct PrimitiveType : Type
{
    PrimitiveTypeKind kind;

    explicit PrimitiveType(const PrimitiveTypeKind kind)
        : kind(kind)
    {
    }

    [[nodiscard]] type_ptr_t as_shared() const override
    {
        return std::make_shared<PrimitiveType>(*this);
    }

    [[nodiscard]] std::string to_string() const override
    {
        switch (kind)
        {
            case PrimitiveTypeKind::Number:
                return "number";
            case PrimitiveTypeKind::String:
                return "string";
            case PrimitiveTypeKind::Bool:
                return "bool";
            case PrimitiveTypeKind::Void:
                return "void";
        }
        return "???";
    }
};

const auto number_type = PrimitiveType(PrimitiveTypeKind::Number);
const auto string_type = PrimitiveType(PrimitiveTypeKind::String);
const auto bool_type = PrimitiveType(PrimitiveTypeKind::Bool);
const auto void_type = PrimitiveType(PrimitiveTypeKind::Void);