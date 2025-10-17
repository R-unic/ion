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

    [[nodiscard]] type_ptr_t as_shared() const override
    {
        return std::make_shared<InterfaceType>(*this);
    }

    [[nodiscard]] std::string to_string() const override
    {
        return name + ' ' + ObjectType::to_string();
    }
};