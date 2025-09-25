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

void AstViewer::write_line()
{
    write("\n");
}

void AstViewer::write_line(const std::string& text) const
{
    write_line(text.c_str());
}

void AstViewer::write_indent() const
{
    std::cout << std::string(2ull * indent_, ' ');
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
                write(arg);
            else if constexpr (std::is_same_v<type_t, bool>)
                write(arg ? "true" : "false");
        }, literal_value);
    }
    write(")");
}

void AstViewer::visit_binary_op(const BinaryOp& binary_op)
{
    indent_++;
    write_line("BinaryOp(");
    visit(binary_op.left);
    write_line(",");
    write_line('"' + get_text(binary_op.operator_token) + "\",");
    visit(binary_op.right);
    write_line();
    indent_--;
    write_indent();
    write(")");
}

void AstViewer::visit_expression_statement(const ExpressionStatement& expression_statement)
{
    indent_++;
    write_line("ExpressionStatement(");
    visit(expression_statement.expression);
    indent_--;
    write_indent();
    write_line(")");
}