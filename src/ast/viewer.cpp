#include <iostream>
#include <ostream>

#include "ast/viewer.h"

void AstViewer::write(const std::string& text)
{
    write(text.c_str());
}

void AstViewer::write(const char* text)
{
    std::cout << text;
}

void AstViewer::write_indent() const
{
    std::cout << std::string(2ull * indent_, ' ');
}

void AstViewer::write_line() const
{
    write("\n");
    write_indent();
}

void AstViewer::write_line(const std::string& text) const
{
    write_line(text.c_str());
}

void AstViewer::write_line(const char* text) const
{
    std::cout << text << '\n';
    write_indent();
}

void AstViewer::visit_literal(const Literal& literal)
{
    write("Literal(");
    if (!literal.value.has_value())
        write("null");
    else
    {
        const auto literal_value = literal.value.value();
        std::visit([]<typename T>(T& arg)
        {
            using type_t = std::decay_t<T>;
            if constexpr (std::is_same_v<type_t, double>)
                write(std::to_string(arg));
            else if constexpr (std::is_same_v<type_t, std::string>)
                write('"' + arg + '"');
            else if constexpr (std::is_same_v<type_t, bool>)
                write(arg ? "true" : "false");
        }, literal_value);
    }
    write(")");
}

void AstViewer::visit_identifier(const Identifier& identifier)
{
    write("Identifier(");
    write(identifier.name);
    write(")");
}

void AstViewer::write_binary_op_contents(const BinaryOp& binary_op) const
{
    visit(binary_op.left);
    write_line(",");
    write_line('"' + get_text(binary_op.operator_token) + "\",");
    visit(binary_op.right);
}

void AstViewer::visit_binary_op(const BinaryOp& binary_op)
{
    indent_++;
    write_line("BinaryOp(");
    write_binary_op_contents(binary_op);
    write_closing_paren();
}

void AstViewer::visit_unary_op(const UnaryOp& unary_op)
{
    indent_++;
    write_line("UnaryOp(");
    write_line('"' + get_text(unary_op.operator_token) + "\",");
    visit(unary_op.operand);
    write_closing_paren();
}

void AstViewer::visit_assignment_op(const AssignmentOp& assignment_op)
{
    indent_++;
    write_line("AssignmentOp(");
    write_binary_op_contents(assignment_op);
    write_closing_paren();
}

void AstViewer::visit_invocation(const Invocation& invocation)
{
    indent_++;
    write_line("Invocation(");
    visit(invocation.callee);
    write_line(",");

    const auto starting_indent = indent_++;
    write("[");
    
    size_t i = 0;
    for (const auto& argument : *invocation.arguments)
    {
        write_line();
        visit(argument);
        if (++i < invocation.arguments->size())
            write(",");
        else
        {
            indent_--;
            write_line();
        }
    }

    if (indent_ != starting_indent)
        indent_--;
    
    write_line("],");
    write("Special: ");
    write(invocation.bang_token.has_value() ? "true" : "false");
    write_closing_paren();
}

void AstViewer::visit_member_access(const MemberAccess& member_access)
{
    indent_++;
    write_line("MemberAccess(");
    visit(member_access.expression);
    write_line(",");
    write(get_text(member_access.name));
    write_closing_paren();
}

void AstViewer::visit_expression_statement(const ExpressionStatement& expression_statement)
{
    indent_++;
    write_line("ExpressionStatement(");
    visit(expression_statement.expression);
    write_closing_paren();
}

void AstViewer::write_closing_paren()
{
    indent_--;
    write_line();
    write(")");
}

void AstViewer::visit_variable_declaration(const VariableDeclaration& variable_declaration)
{
    indent_++;
    write_line("VariableDeclaration(");
    write(get_text(variable_declaration.name));
    if (variable_declaration.initializer.has_value())
    {
        write_line(",");
        visit(*variable_declaration.initializer);
    }
    
    write_closing_paren();
}

void AstViewer::visit_if(const If& if_statement)
{
    indent_++;
    write_line("If(");
    visit(if_statement.condition);
    write_line(",");
    visit(if_statement.then_branch);
    if (if_statement.else_branch.has_value())
    {
        write_line(",");
        visit(*if_statement.else_branch);
    }
    
    write_closing_paren();
}

void AstViewer::visit_while(const While& while_statement)
{
    indent_++;
    write_line("While(");
    visit(while_statement.condition);
    write_line(",");
    visit(while_statement.body);
    write_closing_paren();
}

void AstViewer::visit_import(const Import& import)
{
    indent_++;
    write_line("Import(");
    
    const auto starting_indent = indent_++;
    write("[");
    
    size_t i = 0;
    for (const auto& argument : import.names)
    {
        write_line();
        write(get_text(argument));
        if (++i < import.names.size())
            write(",");
        else
        {
            indent_--;
            write_line();
        }
    }

    if (indent_ != starting_indent)
        indent_--;
    
    write_line("],");
    write(get_text(import.module_name));
    write_closing_paren();
}