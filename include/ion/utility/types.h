#pragma once
#include "ion/diagnostics.h"
#include "ion/symbols/symbol.h"
#include "ion/types/union_type.h"

#define ASSERT_SYMBOL(symbol) \
    COMPILER_ASSERT(symbol.has_value(), "Symbol does not exist")

#define ASSERT_NODE_SYMBOL(node) \
    COMPILER_ASSERT((node).symbol.has_value(), "Symbol does not exist on node (" + std::string(typeid(node).name()) + ')')

#define ASSERT_DECLARING_SYMBOL(symbol) \
    COMPILER_ASSERT((symbol)->declaring_symbol.has_value(), "Declaring symbol does not exist on symbol (" + std::string(typeid(symbol).name()) + ')')

#define ASSERT_TYPE(symbol) \
    COMPILER_ASSERT((symbol)->type.has_value(), "Type does not exist on symbol (" + std::string(typeid(symbol).name()) + ')')

#define ASSERT_NODE_TYPE(node) \
    ASSERT_SYMBOL(node) \
    ASSERT_TYPE(node.symbol)

inline symbol_ptr_t get_symbol(const SyntaxNode& node)
{
    ASSERT_SYMBOL(node.symbol);
    return *node.symbol;
}

inline type_ptr_t get_type(const SyntaxNode& node)
{
    const auto symbol = get_symbol(node);
    ASSERT_TYPE(symbol);
    return *symbol->type;
}

inline std::vector<type_ptr_t> get_types(const std::vector<expression_ptr_t>& expressions)
{
    std::vector<type_ptr_t> types;
    for (const auto& expression : expressions)
        types.push_back(get_type(*expression));

    return types;
}

inline std::vector<type_ptr_t> get_types(const std::vector<statement_ptr_t>& statements)
{
    std::vector<type_ptr_t> types;
    for (const auto& statement : statements)
        types.push_back(get_type(*statement));

    return types;
}

inline type_ptr_t create_union(const std::vector<type_ptr_t>& types)
{
    auto is_singular = true;
    auto last_type = types.front();

    for (const auto& type : types)
    {
        is_singular = is_singular && last_type->is_same(type);
        last_type = type;
    }

    return is_singular ? last_type : std::make_shared<UnionType>(types);
}