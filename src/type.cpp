#include "ion/ast/ast.h"
#include "ion/types/type.h"

#include "ion/types/all.h"

template <typename To, typename From>
To* dynamic_unique_ptr_cast(std::unique_ptr<From>& from)
{
    return dynamic_cast<To*>(from.get());
}

static PrimitiveTypeKind get_primitive_type_kind(const std::string& primitive_name)
{
    if (primitive_name == "number")
        return PrimitiveTypeKind::Number;
    if (primitive_name == "string")
        return PrimitiveTypeKind::String;
    if (primitive_name == "bool")
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
type_ptr_t from_function_like(const FunctionLike& fn_like)
{
    const auto type_parameters = fn_like->type_parameters.has_value()
                                     ? from_list(fn_like->type_parameters.value()->list)
                                     : std::vector<type_ptr_t>();

    return std::make_shared<FunctionType>(type_parameters, from_list(fn_like->parameter_types), Type::from(fn_like->return_type));
}

type_ptr_t from_type_parameter(TypeParameterRef* type_parameter)
{
    const auto base_type = type_parameter->base_type.has_value()
                               ? Type::from(*type_parameter->base_type)
                               : std::optional<type_ptr_t>(std::nullopt);
    const auto default_type = type_parameter->default_type.has_value()
                                  ? Type::from(*type_parameter->default_type)
                                  : std::optional<type_ptr_t>(std::nullopt);

    return std::make_shared<TypeParameter>(type_parameter->name.get_text(), base_type, default_type);
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
    std::optional<type_ptr_t> result;
    if (const auto primitive_type = dynamic_unique_ptr_cast<PrimitiveTypeRef>(type_ref))
        result = std::make_shared<PrimitiveType>(get_primitive_type_kind(primitive_type->get_text()));
    if (const auto type_name = dynamic_unique_ptr_cast<TypeNameRef>(type_ref))
        result = std::make_shared<TypeName>(type_name->name.get_text());
    if (const auto literal_type = dynamic_unique_ptr_cast<LiteralTypeRef>(type_ref))
        result = std::make_shared<LiteralType>(literal_type->value);
    if (const auto nullable_type = dynamic_unique_ptr_cast<NullableTypeRef>(type_ref))
        result = std::make_shared<NullableType>(from(nullable_type->non_nullable_type));
    if (const auto array_type = dynamic_unique_ptr_cast<ArrayTypeRef>(type_ref))
        result = std::make_shared<ArrayType>(from(array_type->element_type));
    if (const auto tuple_type = dynamic_unique_ptr_cast<TupleTypeRef>(type_ref))
        result = std::make_shared<TupleType>(from_list(tuple_type->element_types));
    if (const auto union_type = dynamic_unique_ptr_cast<UnionTypeRef>(type_ref))
        result = std::make_shared<UnionType>(from_list(union_type->types));
    if (const auto intersection_type = dynamic_unique_ptr_cast<IntersectionTypeRef>(type_ref))
        result = std::make_shared<IntersectionType>(from_list(intersection_type->types));
    // if (const auto object_type = reinterpret_unique_ptr_cast<ObjectTypeRef>(std::move(type_ref)))
    //     result = std::make_shared<ObjectType>(from_list(function_type->parameter_types), from(function_type->return_type));
    if (const auto function_type = dynamic_unique_ptr_cast<FunctionTypeRef>(type_ref))
        result = from_function_like(function_type);
    if (const auto type_parameter = dynamic_unique_ptr_cast<TypeParameterRef>(type_ref))
        result = from_type_parameter(type_parameter);

    if (result.has_value())
        return *result;

    report_compiler_error(std::string("Failed to convert type ref to type: ") + typeid(*type_ref).name());
}

type_ptr_t Type::lower(const type_ptr_t& type)
{
    if (type->is_literal())
        return std::dynamic_pointer_cast<LiteralType>(type)->as_primitive();

    if (type->is_literal_union())
    {
        const auto union_type = std::dynamic_pointer_cast<UnionType>(type);
        auto first = true;
        auto last_type = union_type->types.front();
        std::vector<type_ptr_t> lowered_types;
        for (const auto& subtype : union_type->types)
        {
            const auto lowered = lower(subtype);
            if (!first && last_type->is_same(lowered))
                continue;

            lowered_types.push_back(lowered);
            if (first)
                first = false;

            last_type = lowered;
        }

        return std::make_shared<UnionType>(lowered_types);
    }

    if (type->is_literal_array())
    {
        const auto array_type = std::dynamic_pointer_cast<ArrayType>(type);
        return std::make_shared<ArrayType>(lower(array_type->element_type));
    }

    if (type->is_literal_tuple())
    {
        const auto tuple_type = std::dynamic_pointer_cast<TupleType>(type);
        std::vector<type_ptr_t> lowered_types;
        for (const auto& subtype : tuple_type->element_types)
            lowered_types.push_back(lower(subtype));

        return std::make_shared<TupleType>(lowered_types);
    }

    return type;
}

bool Type::is_list_same(const std::vector<type_ptr_t>& list, const std::vector<type_ptr_t>& other_list)
{
    auto i = 0;
    auto is_same = true;
    for (const auto& type : list)
    {
        const auto& other_type = other_list.at(i++);
        is_same = is_same && type->is_same(other_type);
    }

    return is_same;
}