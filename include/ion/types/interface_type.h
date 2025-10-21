#pragma once
#include <string>

#include "object_type.h"
#include "type.h"
#include "ion/utility/ast.h"

struct InterfaceType final : ObjectType
{
    std::string name;
    std::vector<type_ptr_t> type_parameters;

    explicit InterfaceType(std::string name, member_map_t members, std::vector<type_ptr_t> type_parameters)
        : ObjectType(std::move(members)),
          name(std::move(name)),
          type_parameters(std::move(type_parameters))
    {
    }

    [[nodiscard]] bool is_same(const type_ptr_t& other) const override
    {
        CAST_CHECK(interface, InterfaceType);
        return name == interface->name && ObjectType::is_same(other);
    }

    TYPE_OVERRIDES(interface, InterfaceType);

    [[nodiscard]] std::string to_string() const override
    {
        return name + generics_to_string(type_parameters) + ' ' + ObjectType::to_string();
    }
};