#include "ion/ast/viewer.h"

#include "ion/ast/statements/instance_name_declarator.h"

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

void AstViewer::write_type_parameters(const std::optional<TypeParametersClause*>& type_parameters)
{
    if (!type_parameters.has_value())
        return;

    write_line(",");
    write_list<type_ref_ptr_t>(type_parameters.value()->list, [&](const auto& type_parameter)
    {
        visit(type_parameter);
    });
}

void AstViewer::visit_primitive_literal(const PrimitiveLiteral& literal)
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

void AstViewer::visit_range_literal(const RangeLiteral& range_literal)
{
    indent_++;
    write_line("RangeLiteral(");
    visit(range_literal.minimum);
    write_line(",");
    visit(range_literal.maximum);
    write_closing_paren();
}

void AstViewer::visit_rgb_literal(const RgbLiteral& rgb_literal)
{
    indent_++;
    write_line("RgbLiteral(");
    visit(rgb_literal.r);
    write_line(",");
    visit(rgb_literal.g);
    write_line(",");
    visit(rgb_literal.b);
    write_closing_paren();
}

void AstViewer::visit_hsv_literal(const HsvLiteral& hsv_literal)
{
    indent_++;
    write_line("HsvLiteral(");
    visit(hsv_literal.h);
    write_line(",");
    visit(hsv_literal.s);
    write_line(",");
    visit(hsv_literal.v);
    write_closing_paren();
}

void AstViewer::visit_vector_literal(const VectorLiteral& vector_literal)
{
    indent_++;
    write_line("VectorLiteral(");
    visit(vector_literal.x);
    write_line(",");
    visit(vector_literal.y);
    write_line(",");
    visit(vector_literal.z);
    write_closing_paren();
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
    write_list<expression_ptr_t>(invocation.arguments, [&](const auto& argument)
    {
        visit(argument);
    });
    write_line(",");

    write("Special: ");
    write(invocation.bang_token.has_value() ? "true" : "false");
    write_closing_paren();
}

void AstViewer::visit_name_of(const NameOf& name_of)
{
    write("NameOf(");
    write(name_of.identifier.get_text());
    write(")");
}

void AstViewer::visit_type_of(const TypeOf& type_of)
{
    indent_++;
    write_line("TypeOf(");
    visit(type_of.expression);
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
    write_list<statement_ptr_t>(block.statements, [&](const auto& statement)
    {
        visit(statement);
    });
    write(")");
}

void AstViewer::visit_type_declaration(const TypeDeclaration& type_declaration)
{
    indent_++;
    write_line("TypeDeclaration(");
    write(type_declaration.name.get_text());
    write_type_parameters(type_declaration.type_parameters);
    write_line(",");
    visit(type_declaration.type);
    write_closing_paren();
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
    if (variable_declaration.equals_value.has_value())
    {
        write_line(",");
        visit(variable_declaration.equals_value.value()->value);
    }

    write_closing_paren();
}

void AstViewer::visit_event_declaration(const EventDeclaration& event_declaration)
{
    indent_++;
    write_line("EventDeclaration(");
    write(event_declaration.name.get_text());
    write_type_parameters(event_declaration.type_parameters);
    write_line(",");
    write_list<type_ref_ptr_t>(event_declaration.parameter_types, [&](const auto& parameter_type)
    {
        visit(parameter_type);
    });

    write_closing_paren();
}

void AstViewer::visit_enum_declaration(const EnumDeclaration& enum_declaration)
{
    indent_++;
    write_line("EnumDeclaration(");
    write_line(enum_declaration.name.get_text() + ',');
    write_list<statement_ptr_t>(enum_declaration.members, [&](const auto& member)
    {
        visit(member);
    });
    write_closing_paren();
}

void AstViewer::visit_enum_member(const EnumMember& enum_member)
{
    const auto has_initializer = enum_member.equals_value.has_value();
    if (has_initializer)
        indent_++;

    write("EnumMember(");
    if (has_initializer)
        write_line();

    write(enum_member.name.get_text());
    if (!has_initializer)
        write(")");
    else
    {
        write_line(",");
        visit(enum_member.equals_value.value()->value);
        write_closing_paren();
    }
}

void AstViewer::visit_function_declaration(const FunctionDeclaration& function_declaration)
{
    indent_++;
    write_line("FunctionDeclaration(");
    write(function_declaration.name.get_text());
    write_type_parameters(function_declaration.type_parameters);
    write_line(",");
    write_list<statement_ptr_t>(function_declaration.parameters, [&](const auto& parameter_type)
    {
        visit(parameter_type);
    });

    if (function_declaration.return_type.has_value())
    {
        write_line(",");
        visit(*function_declaration.return_type);
    }

    write_line(",");
    if (function_declaration.expression_body.has_value())
        visit(*function_declaration.expression_body);
    else
        visit(*function_declaration.body);

    write_closing_paren();
}

void AstViewer::visit_parameter(const Parameter& parameter)
{
    indent_++;
    write_line("Parameter(");
    write(parameter.name.get_text());
    if (parameter.type.has_value())
    {
        write_line(",");
        visit(*parameter.type);
    }
    if (parameter.equals_value.has_value())
    {
        write_line(",");
        visit(parameter.equals_value.value()->value);
    }

    write_closing_paren();
}

void AstViewer::visit_instance_constructor(const InstanceConstructor& instance_constructor)
{
    indent_++;
    write_line("InstanceConstructor(");
    write_line(instance_constructor.name.get_text() + ", ");
    visit(instance_constructor.type);
    write_line(",");
    write_list<statement_ptr_t>(instance_constructor.declarators, [&](const auto& property_declarator)
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

void AstViewer::visit_instance_name_declarator(const InstanceNameDeclarator& instance_name_declarator)
{
    write("InstanceNameDeclarator(");
    write(instance_name_declarator.name.get_text());
    write(")");
}

void AstViewer::visit_instance_attribute_declarator(const InstanceAttributeDeclarator& instance_attribute_declarator)
{
    write("InstanceAttributeDeclarator(");
    write(instance_attribute_declarator.name.get_text() + ", ");
    visit(instance_attribute_declarator.value);
    write(")");
}

void AstViewer::visit_instance_tag_declarator(const InstanceTagDeclarator& instance_tag_declarator)
{
    write("InstanceTagDeclarator(");
    write(instance_tag_declarator.name.get_text());
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

void AstViewer::visit_union_type(const UnionType& union_type)
{
    indent_++;
    write_line("UnionType(");
    write_list<type_ref_ptr_t>(union_type.types, [&](const auto& type_ref)
    {
        visit(type_ref);
    });
    write_closing_paren();
}

void AstViewer::visit_intersection_type(const IntersectionType& intersection_type)
{
    indent_++;
    write_line("IntersectionType(");
    write_list<type_ref_ptr_t>(intersection_type.types, [&](const auto& type_ref)
    {
        visit(type_ref);
    });
    write_closing_paren();
}