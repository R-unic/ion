#pragma once
#include <memory>
#include <optional>

#include "../types/type.h"

struct Symbol;
using symbol_ptr_t = std::shared_ptr<Symbol>;

struct Symbol
{
    std::optional<type_ptr_t> type;
    std::optional<symbol_ptr_t> declaring_symbol;

    explicit Symbol(std::optional<type_ptr_t> type = std::nullopt)
        : type(std::move(type))
    {
    }

    virtual ~Symbol() = default;
    Symbol(const Symbol&) = delete;
    Symbol& operator=(const Symbol&) = delete;
    Symbol(Symbol&&) noexcept = default;
    Symbol& operator=(Symbol&&) noexcept = default;

    type_ptr_t get_type();

    [[nodiscard]] std::string declaring_symbol_to_string() const
    {
        return declaring_symbol.has_value() ? " < " + declaring_symbol.value()->to_string() : "";
    }

    [[nodiscard]] virtual std::string to_string() const
    {
        return "Symbol(" + (type.has_value() ? type.value()->to_string() : "") + ')' + declaring_symbol_to_string();
    }

    [[nodiscard]] virtual bool is_named_symbol() const
    {
        return false;
    }

    [[nodiscard]] virtual bool is_declaration_symbol() const
    {
        return false;
    }

    [[nodiscard]] virtual bool is_type_symbol() const
    {
        return false;
    }

    [[nodiscard]] virtual bool is_type_declaration_symbol() const
    {
        return false;
    }
};

