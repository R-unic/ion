#pragma once

#include "visitor.h"

class AstViewer final : public ExpressionVisitor<void>, public StatementVisitor<void>
{
    unsigned int indent_ = 0;
    
    static void write(const std::string&);
    static void write(const char*);
    static void write_line();
    void write_indent() const;
    void write_line(const std::string&) const;
    void write_line(const char*) const;
    
public:
    AstViewer() = default;
    
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
    
    void write_binary_op_contents(const BinaryOp& binary_op) const;
    
    void visit_literal(const Literal&) override;
    void visit_identifier(const Identifier&) override;
    void visit_binary_op(const BinaryOp&) override;
    void visit_unary_op(const UnaryOp&) override;
    void visit_assignment_op(const AssignmentOp&) override;
    void visit_invocation(const Invocation&) override;
    void visit_member_access(const MemberAccess&) override;
    
    void visit_expression_statement(const ExpressionStatement&) override;
    void visit_variable_declaration(const VariableDeclaration&) override;
    void visit_if(const If&) override;
    void visit_while(const While&) override;
    void visit_import(const Import&) override;
};