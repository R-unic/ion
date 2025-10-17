#pragma once
#include <memory>
#include <string>

#include "ion/utility/basic.h"

struct Type;
class TypeRef;
using type_ptr_t = std::shared_ptr<Type>;

struct Type
{
    static type_ptr_t from_interface(const InterfaceDeclaration& declaration);
    static type_ptr_t from(std::unique_ptr<TypeRef>&);

    [[nodiscard]] virtual bool is_literal() const
    {
        return false;
    }

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