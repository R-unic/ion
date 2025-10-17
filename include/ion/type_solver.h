#pragma once
#include "ion/ast/visitor.h"
#include "ion/logger.h"

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

class TypeSolver final : public AstVisitor<void>
{
public:
    TypeSolver()
    {
        logger::info("Created type solver");
    }

    static void bind_type(const SyntaxNode&, const type_ptr_t&);

    void visit_primitive_literal(PrimitiveLiteral&) override;
    void visit_identifier(Identifier&) override;

    void visit_expression_statement(ExpressionStatement&) override;
    void visit_variable_declaration(VariableDeclaration&) override;
};