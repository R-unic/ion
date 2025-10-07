#include "ion/resolver.h"

#include "ion/ast/statements/decorator.h"

void Resolver::visit_primitive_literal(const PrimitiveLiteral&)
{
}

void Resolver::visit_array_literal(const ArrayLiteral& array_literal)
{
    for (const auto& element : array_literal.elements)
        visit(element);
}

void Resolver::visit_tuple_literal(const TupleLiteral& tuple_literal)
{
    for (const auto& element : tuple_literal.elements)
        visit(element);
}

void Resolver::visit_range_literal(const RangeLiteral& range_literal)
{
    visit(range_literal.minimum);
    visit(range_literal.maximum);
}

void Resolver::visit_rgb_literal(const RgbLiteral& rgb_literal)
{
    visit(rgb_literal.r);
    visit(rgb_literal.g);
    visit(rgb_literal.b);
}

void Resolver::visit_hsv_literal(const HsvLiteral& hsv_literal)
{
    visit(hsv_literal.h);
    visit(hsv_literal.s);
    visit(hsv_literal.v);
}

void Resolver::visit_vector_literal(const VectorLiteral& vector_literal)
{
    visit(vector_literal.x);
    visit(vector_literal.y);
    visit(vector_literal.z);
}

void Resolver::visit_interpolated_string(const InterpolatedString& interpolated_string)
{
    for (const auto& expression : interpolated_string.interpolations)
        visit(expression);
}

void Resolver::visit_identifier(const Identifier&)
{
}

void Resolver::visit_parenthesized(const Parenthesized& parenthesized)
{
    visit(parenthesized.expression);
}

void Resolver::visit_binary_op(const BinaryOp& binary_op)
{
    visit(binary_op.left);
    visit(binary_op.right);
}

void Resolver::visit_unary_op(const UnaryOp&)
{
}

void Resolver::visit_postfix_unary_op(const PostfixUnaryOp& postfix_unary_op)
{
    visit(postfix_unary_op.operand);
}

void Resolver::visit_assignment_op(const AssignmentOp&)
{
}

void Resolver::visit_ternary_op(const TernaryOp&)
{
}

void Resolver::visit_invocation(const Invocation&)
{
}

void Resolver::visit_type_of(const TypeOf& type_of)
{
    visit(type_of.expression);
}

void Resolver::visit_name_of(const NameOf&)
{
}

void Resolver::visit_await(const Await& await)
{
    visit(await.expression);
}

void Resolver::visit_member_access(const MemberAccess& member_access)
{
    visit(member_access.expression);
}

void Resolver::visit_optional_member_access(const OptionalMemberAccess& optional_member_access)
{
    visit(optional_member_access.expression);
}

void Resolver::visit_element_access(const ElementAccess& element_access)
{
    visit(element_access.expression);
    visit(element_access.index_expression);
}

void Resolver::visit_expression_statement(const ExpressionStatement& expression_statement)
{
    visit(expression_statement.expression);
}

void Resolver::visit_block(const Block& block)
{
    visit_statements(block.braced_statement_list->statements);
}

void Resolver::visit_type_declaration(const TypeDeclaration&)
{
}

void Resolver::visit_variable_declaration(const VariableDeclaration& variable_declaration)
{
    if (variable_declaration.equals_value.has_value())
        visit(variable_declaration.equals_value.value()->value);
}

void Resolver::visit_event_declaration(const EventDeclaration&)
{
}

void Resolver::visit_interface_declaration(const InterfaceDeclaration&)
{
}

void Resolver::visit_interface_field(const InterfaceField&)
{
}

void Resolver::visit_interface_method(const InterfaceMethod&)
{
}

void Resolver::visit_enum_declaration(const EnumDeclaration&)
{
}

void Resolver::visit_enum_member(const EnumMember&)
{
}

void Resolver::visit_function_declaration(const FunctionDeclaration& function_declaration)
{
    if (function_declaration.body->block.has_value())
        visit(*function_declaration.body->block);
    else
        visit(function_declaration.body->expression_body.value()->expression);
}

void Resolver::visit_parameter(const Parameter&)
{
}

void Resolver::visit_instance_constructor(const InstanceConstructor&)
{
}

void Resolver::visit_instance_property_declarator(const InstancePropertyDeclarator&)
{
}

void Resolver::visit_instance_name_declarator(const InstanceNameDeclarator&)
{
}

void Resolver::visit_instance_attribute_declarator(const InstanceAttributeDeclarator&)
{
}

void Resolver::visit_instance_tag_declarator(const InstanceTagDeclarator&)
{
}

void Resolver::visit_break(const Break&)
{
}

void Resolver::visit_continue(const Continue&)
{
}

void Resolver::visit_return(const Return&)
{
}

void Resolver::visit_if(const If& if_statement)
{
    visit(if_statement.condition);
    visit(if_statement.then_branch);
    if (if_statement.else_branch.has_value())
        visit(*if_statement.else_branch);
}

void Resolver::visit_while(const While& while_statement)
{
    visit(while_statement.condition);
    visit(while_statement.statement);
}

void Resolver::visit_repeat(const Repeat& repeat_statement)
{
    visit(repeat_statement.statement);
    visit(repeat_statement.condition);
}

void Resolver::visit_for(const For& for_statement)
{
    visit(for_statement.iterable);
    visit(for_statement.statement);
}

void Resolver::visit_after(const After& after_statement)
{
    visit(after_statement.time_expression);
    visit(after_statement.statement);
}

void Resolver::visit_every(const Every& every_statement)
{
    visit(every_statement.time_expression);
    visit(every_statement.statement);
}

void Resolver::visit_match(const Match& match_statement)
{
    visit(match_statement.expression);
    visit_statements(match_statement.cases->statements);
}

void Resolver::visit_match_case(const MatchCase& match_case)
{
    for (const auto& expression : match_case.comparands)
        visit(expression);

    visit(match_case.statement);
}

void Resolver::visit_match_else_case(const MatchElseCase& match_else_case)
{
    visit(match_else_case.statement);
}

void Resolver::visit_import(const Import&)
{
}

void Resolver::visit_export(const Export& export_statement)
{
    visit(export_statement.statement);
}

void Resolver::visit_decorator(const Decorator& decorator)
{
    for (const auto& expression : decorator.arguments)
        visit(expression);
}

void Resolver::visit_primitive_type(const PrimitiveTypeRef&)
{
}

void Resolver::visit_literal_type(const LiteralTypeRef&)
{
}

void Resolver::visit_type_name(const TypeNameRef&)
{
}

void Resolver::visit_nullable_type(const NullableTypeRef&)
{
}

void Resolver::visit_array_type(const ArrayTypeRef&)
{
}

void Resolver::visit_tuple_type(const TupleTypeRef&)
{
}

void Resolver::visit_function_type(const FunctionTypeRef&)
{
}

void Resolver::visit_union_type(const UnionTypeRef&)
{
}

void Resolver::visit_intersection_type(const IntersectionTypeRef&)
{
}

void Resolver::visit_type_parameter(const TypeParameterRef&)
{
}