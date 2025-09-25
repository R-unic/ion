#pragma once

#include "ast/visitor.h"
#include "ast/expressions/binary_op.h"
#include "ast/expressions/literal.h"
#include "ast/statements/expression_statement.h"

class AstViewer final : public ExpressionVisitor<void>, public StatementVisitor<void>
{
    unsigned int indent_ = 0;
    
    static void write(const std::string&);
    static void write(const char*);
    static void write_line();
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
    
    void visit_literal(const Literal&) override;
    void visit_binary_op(const BinaryOp&) override;
    
    void visit_expression_statement(const ExpressionStatement&) override;
};