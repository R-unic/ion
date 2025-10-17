#pragma once
#include <string>

#include "object_type.h"
#include "type.h"

struct InterfaceType final : ObjectType
{
    std::string name;

    explicit InterfaceType(std::string name, member_map_t members)
        : ObjectType(std::move(members)),
          name(std::move(name))
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
        return name + ' ' + ObjectType::to_string();
    }
};