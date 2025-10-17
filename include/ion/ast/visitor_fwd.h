#pragma once
class PrimitiveLiteral;
class ArrayLiteral;
class TupleLiteral;
class RangeLiteral;
class RgbLiteral;
class HsvLiteral;
class VectorLiteral;
class InterpolatedString;
class Identifier;
class Parenthesized;
class BinaryOp;
class PostfixUnaryOp;
class UnaryOp;
class AssignmentOp;
class TernaryOp;
class Invocation;
class TypeOf;
class NameOf;
class Await;
class MemberAccess;
class OptionalMemberAccess;
class ElementAccess;

class ExpressionStatement;
class Block;
class VariableDeclaration;
class TypeDeclaration;
class EventDeclaration;
class InterfaceDeclaration;
class InterfaceField;
class InterfaceMethod;
class EnumDeclaration;
class EnumMember;
class FunctionDeclaration;
class Parameter;
class InstanceConstructor;
class InstancePropertyDeclarator;
class InstanceNameDeclarator;
class InstanceAttributeDeclarator;
class InstanceTagDeclarator;
class Break;
class Continue;
class Return;
class If;
class While;
class Repeat;
class For;
class After;
class Every;
class Match;
class MatchCase;
class MatchElseCase;
class Import;
class Export;
class Decorator;

class PrimitiveTypeRef;
class LiteralTypeRef;
class TypeNameRef;
class NullableTypeRef;
class ArrayTypeRef;
class TupleTypeRef;
class FunctionTypeRef;
class IntersectionTypeRef;
class UnionTypeRef;
class TypeParameterRef;

template <typename R>
struct ExpressionVisitor
{
    virtual R visit_primitive_literal(PrimitiveLiteral&) = 0;
    virtual R visit_array_literal(ArrayLiteral&) = 0;
    virtual R visit_tuple_literal(TupleLiteral&) = 0;
    virtual R visit_range_literal(RangeLiteral&) = 0;
    virtual R visit_rgb_literal(RgbLiteral&) = 0;
    virtual R visit_hsv_literal(HsvLiteral&) = 0;
    virtual R visit_vector_literal(VectorLiteral&) = 0;
    virtual R visit_interpolated_string(InterpolatedString&) = 0;
    virtual R visit_identifier(Identifier&) = 0;
    virtual R visit_parenthesized(Parenthesized&) = 0;
    virtual R visit_binary_op(BinaryOp&) = 0;
    virtual R visit_unary_op(UnaryOp&) = 0;
    virtual R visit_postfix_unary_op(PostfixUnaryOp&) = 0;
    virtual R visit_assignment_op(AssignmentOp&) = 0;
    virtual R visit_ternary_op(TernaryOp&) = 0;
    virtual R visit_invocation(Invocation&) = 0;
    virtual R visit_type_of(TypeOf&) = 0;
    virtual R visit_name_of(NameOf&) = 0;
    virtual R visit_await(Await&) = 0;
    virtual R visit_member_access(MemberAccess&) = 0;
    virtual R visit_optional_member_access(OptionalMemberAccess&) = 0;
    virtual R visit_element_access(ElementAccess&) = 0;

    virtual ~ExpressionVisitor() = default;
};

template <typename R>
struct StatementVisitor
{
    virtual R visit_expression_statement(ExpressionStatement&) = 0;
    virtual R visit_block(Block&) = 0;
    virtual R visit_variable_declaration(VariableDeclaration&) = 0;
    virtual R visit_type_declaration(TypeDeclaration&) = 0;
    virtual R visit_event_declaration(EventDeclaration&) = 0;
    virtual R visit_interface_declaration(InterfaceDeclaration&) = 0;
    virtual R visit_interface_field(InterfaceField&) = 0;
    virtual R visit_interface_method(InterfaceMethod&) = 0;
    virtual R visit_enum_declaration(EnumDeclaration&) = 0;
    virtual R visit_enum_member(EnumMember&) = 0;
    virtual R visit_function_declaration(FunctionDeclaration&) = 0;
    virtual R visit_parameter(Parameter&) = 0;
    virtual R visit_instance_constructor(InstanceConstructor&) = 0;
    virtual R visit_instance_property_declarator(InstancePropertyDeclarator&) = 0;
    virtual R visit_instance_name_declarator(InstanceNameDeclarator&) = 0;
    virtual R visit_instance_attribute_declarator(InstanceAttributeDeclarator&) = 0;
    virtual R visit_instance_tag_declarator(InstanceTagDeclarator&) = 0;
    virtual R visit_break(Break&) = 0;
    virtual R visit_continue(Continue&) = 0;
    virtual R visit_return(Return&) = 0;
    virtual R visit_if(If&) = 0;
    virtual R visit_while(While&) = 0;
    virtual R visit_repeat(Repeat&) = 0;
    virtual R visit_for(For&) = 0;
    virtual R visit_after(After&) = 0;
    virtual R visit_every(Every&) = 0;
    virtual R visit_match(Match&) = 0;
    virtual R visit_match_case(MatchCase&) = 0;
    virtual R visit_match_else_case(MatchElseCase&) = 0;
    virtual R visit_import(Import&) = 0;
    virtual R visit_export(Export&) = 0;
    virtual R visit_decorator(Decorator&) = 0;

    virtual ~StatementVisitor() = default;
};

template <typename R>
struct TypeRefVisitor
{
    virtual R visit_primitive_type(PrimitiveTypeRef&) = 0;
    virtual R visit_literal_type(LiteralTypeRef&) = 0;
    virtual R visit_type_name(TypeNameRef&) = 0;
    virtual R visit_nullable_type(NullableTypeRef&) = 0;
    virtual R visit_array_type(ArrayTypeRef&) = 0;
    virtual R visit_tuple_type(TupleTypeRef&) = 0;
    virtual R visit_function_type(FunctionTypeRef&) = 0;
    virtual R visit_type_parameter(TypeParameterRef&) = 0;
    virtual R visit_union_type(UnionTypeRef&) = 0;
    virtual R visit_intersection_type(IntersectionTypeRef&) = 0;

    virtual ~TypeRefVisitor() = default;
};