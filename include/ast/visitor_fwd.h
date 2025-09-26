#pragma once

class Literal;
class Identifier;
class Parenthesized;
class BinaryOp;
class PostfixUnaryOp;
class UnaryOp;
class AssignmentOp;
class TernaryOp;
class Invocation;
class MemberAccess;
class ElementAccess;

class ExpressionStatement;
class Block;
class VariableDeclaration;
class Break;
class Continue;
class Return;
class If;
class While;
class Import;
class Export;

class PrimitiveType;
class NullableType;
class TypeName;

template <typename R>
struct ExpressionVisitor
{
    virtual R visit_literal(const Literal&) = 0;
    virtual R visit_identifier(const Identifier&) = 0;
    virtual R visit_parenthesized(const Parenthesized&) = 0;
    virtual R visit_binary_op(const BinaryOp&) = 0;
    virtual R visit_unary_op(const UnaryOp&) = 0;
    virtual R visit_postfix_unary_op(const PostfixUnaryOp&) = 0;
    virtual R visit_assignment_op(const AssignmentOp&) = 0;
    virtual R visit_ternary_op(const TernaryOp&) = 0;
    virtual R visit_invocation(const Invocation&) = 0;
    virtual R visit_member_access(const MemberAccess&) = 0;
    virtual R visit_element_access(const ElementAccess&) = 0;
    
    virtual ~ExpressionVisitor() = default;
};

template <typename R>
struct StatementVisitor
{
    virtual R visit_expression_statement(const ExpressionStatement&) = 0;
    virtual R visit_block(const Block&) = 0;
    virtual R visit_variable_declaration(const VariableDeclaration&) = 0;
    virtual R visit_break(const Break&) = 0;
    virtual R visit_continue(const Continue&) = 0;
    virtual R visit_return(const Return&) = 0;
    virtual R visit_if(const If&) = 0;
    virtual R visit_while(const While&) = 0;
    virtual R visit_import(const Import&) = 0;
    virtual R visit_export(const Export&) = 0;

    virtual ~StatementVisitor() = default;
};

template <typename R>
struct TypeRefVisitor
{
    virtual R visit_primitive_type(const PrimitiveType&) = 0;
    virtual R visit_type_name(const TypeName& type_name) = 0;
    virtual R visit_nullable_type(const NullableType& nullable) = 0;

    virtual ~TypeRefVisitor() = default;
};