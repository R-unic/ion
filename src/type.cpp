#include "ion/types/type.h"
#include "ion/types/primitive_type.h"
#include "ion/ast/ast.h"
#include "ion/types/function_type.h"

template <typename To, typename From>
static std::unique_ptr<To> reinterpret_unique_ptr_cast(std::unique_ptr<From>&& from)
{
    return std::unique_ptr<To>(reinterpret_cast<To*>(from.release()));
}

static PrimitiveTypeKind get_primitive_type_kind(const std::unique_ptr<PrimitiveTypeRef>& primitive)
{
    const auto text = primitive->get_text();
    if (text == "number")
        return PrimitiveTypeKind::Number;
    if (text == "string")
        return PrimitiveTypeKind::String;
    if (text == "bool")
        return PrimitiveTypeKind::Bool;

    return PrimitiveTypeKind::Void;
}

static std::vector<type_ptr_t> from_list(std::vector<type_ref_ptr_t>& list)
{
    std::vector<type_ptr_t> result;
    for (auto& type_ref : list)
        result.push_back(Type::from(type_ref));

    return result;
}

type_ptr_t Type::from(type_ref_ptr_t& type_ref)
{
    if (const auto primitive_type = reinterpret_unique_ptr_cast<PrimitiveTypeRef>(std::move(type_ref)))
        return std::make_unique<PrimitiveType>(get_primitive_type_kind(primitive_type));

    if (const auto function_type = reinterpret_unique_ptr_cast<FunctionTypeRef>(std::move(type_ref)))
        return std::make_unique<FunctionType>(from_list(function_type->parameter_types), from(function_type->return_type));

    report_compiler_error("Failed to convert type ref to type");
}