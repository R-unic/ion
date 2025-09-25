#pragma once

class Literal;
class Identifier;
class BinaryOp;
class UnaryOp;
class AssignmentOp;
class Invocation;
class MemberAccess;

class ExpressionStatement;
class VariableDeclaration;
class If;
class While;
class Import;

template <typename R>
struct ExpressionVisitor
{
    virtual R visit_literal(const Literal&) = 0;
    virtual R visit_identifier(const Identifier&) = 0;
    virtual R visit_binary_op(const BinaryOp&) = 0;
    virtual R visit_unary_op(const UnaryOp&) = 0;
    virtual R visit_assignment_op(const AssignmentOp&) = 0;
    virtual R visit_invocation(const Invocation&) = 0;
    virtual R visit_member_access(const MemberAccess&) = 0;
    
    virtual ~ExpressionVisitor() = default;
};

template <typename R>
struct StatementVisitor
{
    virtual R visit_expression_statement(const ExpressionStatement&) = 0;
    virtual R visit_variable_declaration(const VariableDeclaration&) = 0;
    virtual R visit_if(const If&) = 0;
    virtual R visit_while(const While&) = 0;
    virtual R visit_import(const Import&) = 0;

    virtual ~StatementVisitor() = default;
};