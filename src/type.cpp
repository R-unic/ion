#include "ion/ast/ast.h"
#include "ion/types/type.h"

#include "ion/logger.h"
#include "ion/types/primitive_type.h"
#include "ion/types/function_type.h"
#include "ion/types/interface_type.h"
#include "ion/types/intersection_type.h"
#include "ion/types/literal_type.h"
#include "ion/types/nullable_type.h"
#include "ion/types/type_name.h"
#include "ion/types/type_parameter.h"
#include "ion/types/union_type.h"

template <typename To, typename From>
std::unique_ptr<To> dynamic_unique_ptr_cast(std::unique_ptr<From>& from)
{
    if (auto* ptr = dynamic_cast<To*>(from.get()))
    {
        from.release();
        return std::unique_ptr<To>(ptr);
    }
    return nullptr;
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
std::shared_ptr<FunctionType> from_function_like(const FunctionLike& fn_like)
{
    const auto type_parameters = fn_like->type_parameters.has_value()
                                     ? from_list(fn_like->type_parameters.value()->list)
                                     : std::vector<type_ptr_t>();

    return std::make_shared<FunctionType>(type_parameters, from_list(fn_like->parameter_types), Type::from(fn_like->return_type));
}

type_ptr_t Type::from_interface(const InterfaceDeclaration& declaration)
{
    InterfaceType::member_map_t members;
    for (auto& member : declaration.members->statements)
        if (const auto field = dynamic_unique_ptr_cast<InterfaceField>(member))
            members.insert_or_assign(std::make_shared<LiteralType>(field->name.get_text()),
                                     from(field->type));
        else if (const auto method = dynamic_unique_ptr_cast<InterfaceMethod>(member))
            members.insert_or_assign(std::make_shared<LiteralType>(method->name.get_text()),
                                     from_function_like(method));

    return std::make_unique<InterfaceType>(declaration.name.get_text(), members);
}

type_ptr_t Type::from(type_ref_ptr_t& type_ref)
{
    if (const auto primitive_type = dynamic_unique_ptr_cast<PrimitiveTypeRef>(type_ref))
        return std::make_shared<PrimitiveType>(get_primitive_type_kind(primitive_type));
    if (const auto type_name = dynamic_unique_ptr_cast<TypeNameRef>(type_ref))
        return std::make_shared<TypeName>(type_name->name.get_text());
    if (const auto literal_type = dynamic_unique_ptr_cast<LiteralTypeRef>(type_ref))
        return std::make_shared<LiteralType>(literal_type->value);
    if (const auto nullable_type = dynamic_unique_ptr_cast<NullableTypeRef>(type_ref))
        return std::make_shared<NullableType>(from(nullable_type->non_nullable_type));
    if (const auto union_type = dynamic_unique_ptr_cast<UnionTypeRef>(type_ref))
        return std::make_shared<UnionType>(from_list(union_type->types));
    if (const auto intersection_type = dynamic_unique_ptr_cast<IntersectionTypeRef>(type_ref))
        return std::make_shared<IntersectionType>(from_list(intersection_type->types));
    // if (const auto object_type = reinterpret_unique_ptr_cast<ObjectTypeRef>(std::move(type_ref)))
    //     return std::make_shared<ObjectType>(from_list(function_type->parameter_types), from(function_type->return_type));
    if (const auto function_type = dynamic_unique_ptr_cast<FunctionTypeRef>(type_ref))
        return from_function_like(function_type);
    if (const auto type_parameter = dynamic_unique_ptr_cast<TypeParameterRef>(type_ref))
    {
        const auto base_type = type_parameter->base_type.has_value()
                                   ? from(*type_parameter->base_type)
                                   : std::optional<type_ptr_t>(std::nullopt);
        const auto default_type = type_parameter->default_type.has_value()
                                      ? from(*type_parameter->default_type)
                                      : std::optional<type_ptr_t>(std::nullopt);

        return std::make_shared<TypeParameter>(type_parameter->name.get_text(), base_type, default_type);
    }

    report_compiler_error(std::string("Failed to convert type ref to type: ") + typeid(*type_ref).name());
}