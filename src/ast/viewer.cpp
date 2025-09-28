#include "ion/ast/viewer.h"

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

void AstViewer::write_binary_op_contents(const BinaryOp& binary_op) const
{
    visit(binary_op.left);
    write_line(",");
    write_line('"' + binary_op.operator_token.get_text() + "\",");
    visit(binary_op.right);
}

void AstViewer::write_closing_paren()
{
    indent_--;
    write_line();
    write(")");
}

template<typename T>
void AstViewer::write_list(const std::vector<T>& list, const std::function<void (const T&)>& write_item)
{
    const auto starting_indent = indent_++;
    write("[");

    const auto size = list.size();
    size_t i = 0;
    for (const auto& item : list)
    {
        write_line();
        write_item(item);
        if (++i < size)
            write(",");
        else
        {
            indent_--;
            write_line();
        }
    }

    if (indent_ != starting_indent)
        indent_--;

    write("]");
}

void AstViewer::visit_literal(const Literal& literal)
{
    write("Literal(");
    if (!literal.value.has_value())
        write("null");
    else
    {
        const auto literal_value = literal.value.value();
        write(literal.get_text());
    }
    write(")");
}

void AstViewer::visit_identifier(const Identifier& identifier)
{
    write("Identifier(");
    write(identifier.name.get_text());
    write(")");
}

void AstViewer::visit_parenthesized(const Parenthesized& parenthesized)
{
    indent_++;
    write_line("Parenthesized(");
    visit(parenthesized.expression);
    write_closing_paren();
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
    write_line('"' + unary_op.operator_token.get_text() + "\",");
    visit(unary_op.operand);
    write_closing_paren();
}

void AstViewer::visit_postfix_unary_op(const PostfixUnaryOp& postfix_unary_op)
{
    indent_++;
    write_line("PostfixUnaryOp(");
    visit(postfix_unary_op.operand);
    write_line(",");
    write('"' + postfix_unary_op.operator_token.get_text() + '"');
    write_closing_paren();
}

void AstViewer::visit_assignment_op(const AssignmentOp& assignment_op)
{
    indent_++;
    write_line("AssignmentOp(");
    write_binary_op_contents(assignment_op);
    write_closing_paren();
}

void AstViewer::visit_ternary_op(const TernaryOp& ternary_op)
{
    indent_++;
    write_line("TernaryOp(");
    visit(ternary_op.condition);
    write_line(",");
    visit(ternary_op.when_true);
    write_line(",");
    visit(ternary_op.when_false);
    write_closing_paren();
}

void AstViewer::visit_invocation(const Invocation& invocation)
{
    indent_++;
    write_line("Invocation(");
    visit(invocation.callee);
    write_line(",");
    write_list<expression_ptr_t>(*invocation.arguments, [&](const auto& argument)
    {
        visit(argument);
    });
    write_line(",");

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
    write(member_access.name.get_text());
    write_closing_paren();
}

void AstViewer::visit_element_access(const ElementAccess& element_access)
{
    indent_++;
    write_line("ElementAccess(");
    visit(element_access.expression);
    write_line(",");
    visit(element_access.index_expression);
    write_closing_paren();
}

void AstViewer::visit_expression_statement(const ExpressionStatement& expression_statement)
{
    indent_++;
    write_line("ExpressionStatement(");
    visit(expression_statement.expression);
    write_closing_paren();
}

void AstViewer::visit_block(const Block& block)
{
    write("Block(");
    write_list<statement_ptr_t>(*block.statements, [&](const auto& statement)
    {
        visit(statement);
    });
    write(")");
}

void AstViewer::visit_variable_declaration(const VariableDeclaration& variable_declaration)
{
    indent_++;
    write_line("VariableDeclaration(");
    write(variable_declaration.name.get_text());
    if (variable_declaration.type.has_value())
    {
        write_line(",");
        visit(*variable_declaration.type);
    }
    if (variable_declaration.initializer.has_value())
    {
        write_line(",");
        visit(*variable_declaration.initializer);
    }

    write_closing_paren();
}

void AstViewer::visit_event_declaration(const EventDeclaration& event_declaration)
{
    indent_++;
    write_line("EventDeclaration(");
    write_line(event_declaration.name.get_text() + ',');
    write_list<type_ref_ptr_t>(*event_declaration.type_parameters, [&](const auto& type_parameter)
    {
        visit(type_parameter);
    });
    write_line(",");
    write_list<type_ref_ptr_t>(*event_declaration.parameter_types, [&](const auto& parameter_type)
    {
        visit(parameter_type);
    });

    write_closing_paren();
}

void AstViewer::visit_instance_constructor(const InstanceConstructor& instance_constructor)
{
    indent_++;
    write_line("InstanceConstructor(");
    write_line(instance_constructor.name.get_text() + ", ");
    visit(instance_constructor.type);
    write_line(",");
    write_list<statement_ptr_t>(*instance_constructor.property_declarators, [&](const auto& property_declarator)
    {
        visit(property_declarator);
    });

    if (instance_constructor.parent.has_value())
    {
        write_line(",");
        visit(*instance_constructor.parent);
    }

    write_closing_paren();
}

void AstViewer::visit_instance_property_declarator(const InstancePropertyDeclarator& instance_property_declarator)
{
    write("InstancePropertyDeclarator(");
    write(instance_property_declarator.name.get_text() + ", ");
    visit(instance_property_declarator.value);
    write(")");
}

void AstViewer::visit_break(const Break&)
{
    write("Break");
}

void AstViewer::visit_continue(const Continue&)
{
    write("Continue");
}

void AstViewer::visit_return(const Return& return_statement)
{
    indent_++;
    write("Return");
    if (return_statement.expression.has_value())
    {
        write_line("(");
        visit(*return_statement.expression);
        write_closing_paren();
    }
    else
        write_line();
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
    write_list<Token>(import.names, [](const auto& token)
    {
        write(token.get_text());
    });
    write_line(",");
    write(import.module_name.get_text());
    write_closing_paren();
}


void AstViewer::visit_export(const Export& export_statement)
{
    indent_++;
    write_line("Export(");
    visit(export_statement.statement);
    write_closing_paren();
}

void AstViewer::visit_primitive_type(const PrimitiveType& primitive_type)
{
    write("PrimitiveType(");
    write(primitive_type.keyword.get_text());
    write(")");
}

void AstViewer::visit_type_name(const TypeName& type_name)
{
    write("TypeName(");
    write(type_name.name.get_text());
    write(")");
}

void AstViewer::visit_nullable_type(const NullableType& nullable_type)
{
    indent_++;
    write_line("NullableType(");
    visit(nullable_type.non_nullable_type);
    write_closing_paren();
}

void AstViewer::visit_type_parameter(const TypeParameter& type_parameter)
{
    const auto has_base_type = type_parameter.base_type.has_value();
    if (has_base_type)
        indent_++;

    write("TypeParameter(");
    if (has_base_type)
        write_line();

    write(type_parameter.name.get_text());
    if (!has_base_type)
        write(")");
    else
    {
        write_line(",");
        visit(*type_parameter.base_type);
        write_closing_paren();
    }
}