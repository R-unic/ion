#pragma once
#include <memory>
#include <string>

#include "ion/ast/node.h"
#include "ion/utility/basic.h"

#define AS_SHARED_OVERRIDE(name) \
    [[nodiscard]] type_ptr_t as_shared() const override \
    { \
        return std::make_shared<name>(*this); \
    }

#define TYPE_OVERRIDES(name, name_capitalized) \
    TRUE_IS_FN(name) \
    AS_SHARED_OVERRIDE(name_capitalized)

#define CAST_CHECK(name, name_capitalized) \
    if (!other->is_##name()) return false; \
    const auto name = std::dynamic_pointer_cast<name_capitalized>(other)

struct Type;
class TypeRef;
using type_ptr_t = std::shared_ptr<Type>;

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

struct Type
{
    static type_ptr_t from_interface(const InterfaceDeclaration& declaration);
    static type_ptr_t from(std::unique_ptr<TypeRef>&);
    static type_ptr_t lower(const type_ptr_t&);
    static bool is_list_same(const std::vector<type_ptr_t>& list, const std::vector<type_ptr_t>& other_list);

    DEFINE_IS_FN(primitive);
    DEFINE_IS_FN(literal);
    DEFINE_IS_FN(literal_union);
    DEFINE_IS_FN(literal_array);
    DEFINE_IS_FN(literal_tuple);
    DEFINE_IS_FN(type_name);
    DEFINE_IS_FN(union);
    DEFINE_IS_FN(intersection);
    DEFINE_IS_FN(nullable);
    DEFINE_IS_FN(array);
    DEFINE_IS_FN(tuple);
    DEFINE_IS_FN(type_parameter);
    DEFINE_IS_FN(function);
    DEFINE_IS_FN(object);
    DEFINE_IS_FN(interface);

    [[nodiscard]] bool is_literal_like() const
    {
        return is_literal() || is_literal_union() || is_literal_array() || is_literal_tuple();
    }

    [[nodiscard]] virtual bool is_same(const type_ptr_t& other) const = 0;
    [[nodiscard]] virtual type_ptr_t as_shared() const = 0;
    [[nodiscard]] virtual std::string to_string() const = 0;

    virtual ~Type() = default;
};

inline std::string join_types_by(const std::vector<type_ptr_t>& texts, const std::string& separator)
{
    std::ostringstream oss;
    auto first = true;
    for (auto& s : texts)
    {
        if (!first)
            oss << separator;

        oss << s->to_string();
        first = false;
    }

    return oss.str();
}