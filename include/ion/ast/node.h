#pragma once
#include <memory>

#include "visitor_fwd.h"
#include "ion/token.h"
#include "ion/file_location.h"
#include "ion/symbols/symbol.h"

#define DEFINE_IS_FN(name) \
    [[nodiscard]] virtual bool is_##name() const \
    { \
        return false; \
    }

class SyntaxNode
{
public:
    std::optional<symbol_ptr_t> symbol;

    [[nodiscard]] virtual Token get_first_token() const = 0;
    [[nodiscard]] virtual Token get_last_token() const;
    [[nodiscard]] virtual FileSpan get_span() const;
    [[nodiscard]] virtual std::string get_text() const = 0;
    [[nodiscard]] symbol_ptr_t get_symbol();

    virtual ~SyntaxNode() = default;
};

class Expression : public SyntaxNode
{
public:
    virtual void accept(ExpressionVisitor<void>&) = 0;

    DEFINE_IS_FN(null_literal)
    DEFINE_IS_FN(primitive_literal)
    DEFINE_IS_FN(literal)
    DEFINE_IS_FN(parenthesized)
    DEFINE_IS_FN(assignment)
    DEFINE_IS_FN(assignment_target)
    DEFINE_IS_FN(name_of_target)
};

class Statement : public SyntaxNode
{
public:
    virtual void accept(StatementVisitor<void>&) = 0;

    DEFINE_IS_FN(return)
    DEFINE_IS_FN(block)
};

class TypeRef : public SyntaxNode
{
public:
    virtual void accept(TypeRefVisitor<void>&) = 0;
};

using expression_ptr_t = std::unique_ptr<Expression>;
using statement_ptr_t = std::unique_ptr<Statement>;
using type_ref_ptr_t = std::unique_ptr<TypeRef>;