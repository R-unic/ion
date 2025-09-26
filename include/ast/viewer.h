#pragma once

#include "visitor.h"

class AstViewer final
    : public ExpressionVisitor<void>,
    public StatementVisitor<void>,
    public TypeRefVisitor<void>
{
    unsigned int indent_ = 0;
    
    static void write(const std::string&);
    static void write(const char*);
    void write_indent() const;
    void write_line() const;
    void write_line(const std::string&) const;
    void write_line(const char*) const;
    
public:
    AstViewer() = default;

    void visit(const std::vector<statement_ptr_t>* statements) const
    {
        size_t i = 0;
        for (const auto& statement : *statements)
        {
            visit(statement);
            if (i++ < statements->size())
                write_line();
        }
    }
    
    void visit(const expression_ptr_t& expression) const
    {
        auto viewer = *this;
        expression->accept(viewer);
    }
    
    void visit(const statement_ptr_t& statement) const
    {
        auto viewer = *this;
        statement->accept(viewer);
    }

    void visit(const type_ref_ptr_t& type_ref) const
    {
        auto viewer = *this;
        type_ref->accept(viewer);
    }
    
    void write_binary_op_contents(const BinaryOp& binary_op) const;
    void write_closing_paren();
    
    void visit_literal(const Literal&) override;
    void visit_identifier(const Identifier&) override;
    void visit_parenthesized(const Parenthesized&) override;
    void visit_binary_op(const BinaryOp&) override;
    void visit_unary_op(const UnaryOp&) override;
    void visit_postfix_unary_op(const PostfixUnaryOp&) override;
    void visit_assignment_op(const AssignmentOp&) override;
    void visit_ternary_op(const TernaryOp&) override;
    void visit_invocation(const Invocation&) override;
    void visit_member_access(const MemberAccess&) override;
    void visit_element_access(const ElementAccess&) override;
    
    void visit_expression_statement(const ExpressionStatement&) override;
    void visit_block(const Block&) override;
    void visit_variable_declaration(const VariableDeclaration&) override;
    void visit_break(const Break&) override;
    void visit_continue(const Continue&) override;
    void visit_return(const Return&) override;
    void visit_if(const If&) override;
    void visit_while(const While&) override;
    void visit_import(const Import&) override;
    void visit_export(const Export&) override;

    void visit_primitive_type(const PrimitiveType&) override;
    void visit_type_name(const TypeName&) override;
    void visit_nullable_type(const NullableType&) override;
};