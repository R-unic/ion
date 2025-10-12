#include "ion/ast/ast.h"
#include "ion/types/type.h"
#include "ion/types/primitive_type.h"
#include "ion/types/function_type.h"
#include "ion/types/interface_type.h"
#include "ion/types/intersection_type.h"
#include "ion/types/literal_type.h"
#include "ion/types/nullable_type.h"
#include "ion/types/union_type.h"

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

template <class FunctionLike>
std::shared_ptr<FunctionType> from_function_like(FunctionLike&& method)
{
    return std::make_shared<FunctionType>(from_list(method->parameter_types), Type::from(method->return_type));
}

type_ptr_t Type::from_interface(const InterfaceDeclaration& declaration)
{
    InterfaceType::member_map_t members;
    for (auto& member : declaration.members->statements)
        if (const auto field = reinterpret_unique_ptr_cast<InterfaceField>(std::move(member)))
            members.insert_or_assign(std::make_shared<LiteralType>(field->name.get_text()),
                                     from(field->type));
        else if (auto method = reinterpret_unique_ptr_cast<InterfaceMethod>(std::move(member)))
            members.insert_or_assign(std::make_shared<LiteralType>(method->name.get_text()),
                                     from_function_like(std::move(method)));

    return std::make_unique<InterfaceType>(declaration.name.get_text(), members);
}

type_ptr_t Type::from(type_ref_ptr_t& type_ref)
{
    if (const auto primitive_type = reinterpret_unique_ptr_cast<PrimitiveTypeRef>(std::move(type_ref)))
        return std::make_shared<PrimitiveType>(get_primitive_type_kind(primitive_type));
    if (const auto literal_type = reinterpret_unique_ptr_cast<LiteralTypeRef>(std::move(type_ref)))
        return std::make_shared<LiteralType>(literal_type->value);
    if (const auto nullable_type = reinterpret_unique_ptr_cast<NullableTypeRef>(std::move(type_ref)))
        return std::make_shared<NullableType>(from(nullable_type->non_nullable_type));
    if (const auto union_type = reinterpret_unique_ptr_cast<UnionTypeRef>(std::move(type_ref)))
        return std::make_shared<UnionType>(from_list(union_type->types));
    if (const auto intersection_type = reinterpret_unique_ptr_cast<IntersectionTypeRef>(std::move(type_ref)))
        return std::make_shared<IntersectionType>(from_list(intersection_type->types));
    // if (const auto object_type = reinterpret_unique_ptr_cast<ObjectTypeRef>(std::move(type_ref)))
    //     return std::make_shared<ObjectType>(from_list(function_type->parameter_types), from(function_type->return_type));
    if (const auto function_type = reinterpret_unique_ptr_cast<FunctionTypeRef>(std::move(type_ref)))
        return std::make_shared<FunctionType>(from_list(function_type->parameter_types), from(function_type->return_type));

    report_compiler_error("Failed to convert type ref to type");
}