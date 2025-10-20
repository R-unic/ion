#include <iostream>

#include "ion/type_solver.h"
#include "ion/utility/types.h"
#include "ion/types/all.h"

void TypeSolver::bind_type(const SyntaxNode& node, const type_ptr_t& type)
{
    node.symbol.value()->type = type;
}

void TypeSolver::visit_primitive_literal(PrimitiveLiteral& primitive_literal)
{
    AstVisitor::visit_primitive_literal(primitive_literal);
    auto type = void_type.as_shared();
    if (primitive_literal.value.has_value())
        type = std::make_shared<LiteralType>(*primitive_literal.value);

    bind_type(primitive_literal, type);
}

void TypeSolver::visit_array_literal(ArrayLiteral& array_literal)
{
    AstVisitor::visit_array_literal(array_literal);
    const auto element_type = create_union(get_types(array_literal.elements));
    const auto type = std::make_shared<ArrayType>(element_type);
    bind_type(array_literal, type);
}

// TODO: constant optimizations can be done here
void TypeSolver::visit_range_literal(RangeLiteral& range_literal)
{
    AstVisitor::visit_range_literal(range_literal);
}

void TypeSolver::visit_tuple_literal(TupleLiteral& tuple_literal)
{
    AstVisitor::visit_tuple_literal(tuple_literal);
    const auto types = get_types(tuple_literal.elements);
    const auto type = std::make_shared<TupleType>(types);
    bind_type(tuple_literal, type);
}

void TypeSolver::visit_rgb_literal(RgbLiteral& rgb_literal)
{
    AstVisitor::visit_rgb_literal(rgb_literal);
}

void TypeSolver::visit_hsv_literal(HsvLiteral& hsv_literal)
{
    AstVisitor::visit_hsv_literal(hsv_literal);
}

void TypeSolver::visit_vector_literal(VectorLiteral& vector_literal)
{
    AstVisitor::visit_vector_literal(vector_literal);
}

void TypeSolver::visit_identifier(Identifier& identifier)
{
    AstVisitor::visit_identifier(identifier);
    ASSERT_NODE_SYMBOL(identifier);

    const auto symbol = *identifier.symbol;
    ASSERT_DECLARING_SYMBOL(symbol);

    const auto declaring_symbol = *symbol->declaring_symbol;
    ASSERT_TYPE(declaring_symbol);

    bind_type(identifier, *declaring_symbol->type);
}

void TypeSolver::visit_expression_statement(ExpressionStatement& expression_statement)
{
    AstVisitor::visit_expression_statement(expression_statement);
    ASSERT_NODE_SYMBOL(expression_statement);

    const auto symbol = *expression_statement.symbol;
    ASSERT_SYMBOL(expression_statement.expression->symbol);

    const auto expression_symbol = *expression_statement.expression->symbol;
    ASSERT_TYPE(expression_symbol);

    symbol->type = *expression_symbol->type;
}

void TypeSolver::visit_variable_declaration(VariableDeclaration& variable_declaration)
{
    AstVisitor::visit_variable_declaration(variable_declaration);
    const auto is_const = variable_declaration.const_keyword.has_value();

    type_ptr_t type;
    if (variable_declaration.colon_type.has_value())
        type = Type::from(variable_declaration.colon_type.value()->type);
    else if (variable_declaration.equals_value.has_value() && !variable_declaration.equals_value.value()->value->is_null_literal())
    {
        const auto& initializer = variable_declaration.equals_value.value()->value;
        ASSERT_SYMBOL(initializer->symbol);

        const auto initializer_symbol = *initializer->symbol;
        ASSERT_TYPE(initializer_symbol);

        const auto initializer_type = *initializer_symbol->type;
        const auto keep_constness = is_const && initializer_type->is_literal_like();
        type = keep_constness ? initializer_type : Type::lower(initializer_type);
    }
    else
        report_no_variable_type_or_initializer(variable_declaration.get_span());

    bind_type(variable_declaration, type);
}