#pragma once
#include "symbols/named_symbol.h"
#include "types/function_type.h"
#include "types/primitive_type.h"

static named_symbol_ptr_t create_print_symbol()
{
    // TODO: type pack
    std::vector<type_ptr_t> print_fn_parameters;
    print_fn_parameters.push_back(string_type.as_shared());

    type_ptr_t print_fn_type = std::make_unique<FunctionType>(std::vector<type_ptr_t>(),
                                                              std::move(print_fn_parameters),
                                                              void_type.as_shared());

    return std::make_shared<NamedSymbol>("print", std::move(print_fn_type));
}

const std::vector intrinsic_symbols {
    create_print_symbol()
};