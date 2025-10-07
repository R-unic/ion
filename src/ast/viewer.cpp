#include <iostream>

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

void AstViewer::write_binary_op_contents(const BinaryOp& binary_op)
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

template <typename T>
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

void AstViewer::write_type_list_clause(const std::optional<TypeListClause*>& type_list_clause)
{
    if (!type_list_clause.has_value())
        return;

    write_line(",");
    write_list<type_ref_ptr_t>(type_list_clause.value()->list, [&](const auto& type_node)
    {
        visit(type_node);
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

void AstViewer::visit_array_literal(const ArrayLiteral& array_literal)
{
    write("ArrayLiteral(");
    write_list<expression_ptr_t>(array_literal.elements, [&](const auto& expression)
    {
        visit(expression);
    });
    write(")");
}

void AstViewer::visit_tuple_literal(const TupleLiteral& tuple_literal)
{
    write("TupleLiteral(");
    write_list<expression_ptr_t>(tuple_literal.elements, [&](const auto& expression)
    {
        visit(expression);
    });
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

void AstViewer::visit_interpolated_string(const InterpolatedString& interpolated_string)
{
    indent_++;
    write_line("InterpolatedString(");
    write_list<Token>(interpolated_string.parts, [&](const auto& token)
    {
        const auto text = token.get_text();
        write(text.empty() ? "(empty)" : '"' + text + '"');
    });
    write_line(",");
    write_list<expression_ptr_t>(interpolated_string.interpolations, [&](const auto& expression)
    {
        visit(expression);
    });
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
    write_type_list_clause(invocation.type_arguments);
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

void AstViewer::visit_type_of(const TypeOf& type_of)
{
    indent_++;
    write_line("TypeOf(");
    visit(type_of.expression);
    write_closing_paren();
}

void AstViewer::visit_name_of(const NameOf& name_of)
{
    write("NameOf(");
    write(name_of.identifier.get_text());
    write(")");
}

void AstViewer::visit_await(const Await& await)
{
    indent_++;
    write_line("Await(");
    visit(await.expression);
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

void AstViewer::visit_optional_member_access(const OptionalMemberAccess& optional_member_access)
{
    indent_++;
    write_line("OptionalMemberAccess(");
    visit(optional_member_access.expression);
    write_line(",");
    write(optional_member_access.name.get_text());
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
    write_list<statement_ptr_t>(block.braced_statement_list->statements, [&](const auto& statement)
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
    write_type_list_clause(type_declaration.type_parameters);
    write_line(",");
    visit(type_declaration.type);
    write_closing_paren();
}

void AstViewer::visit_variable_declaration(const VariableDeclaration& variable_declaration)
{
    indent_++;
    write_line("VariableDeclaration(");
    write(variable_declaration.name.get_text());
    if (variable_declaration.colon_type.has_value())
    {
        write_line(",");
        visit(variable_declaration.colon_type.value()->type);
    }
    if (variable_declaration.equals_value.has_value())
    {
        write_line(",");
        visit(variable_declaration.equals_value.value()->value);
    }
    write_line(",");
    write("Const: ");
    write(variable_declaration.const_keyword.has_value() ? "true" : "false");
    write_closing_paren();
}

void AstViewer::visit_event_declaration(const EventDeclaration& event_declaration)
{
    indent_++;
    write_line("EventDeclaration(");
    write(event_declaration.name.get_text());
    write_type_list_clause(event_declaration.type_parameters);
    write_line(",");
    write_list<type_ref_ptr_t>(event_declaration.parameter_types, [&](const auto& parameter_type)
    {
        visit(parameter_type);
    });
    write_closing_paren();
}

void AstViewer::visit_interface_declaration(const InterfaceDeclaration& interface_declaration)
{
    indent_++;
    write_line("InterfaceDeclaration(");
    write_line(interface_declaration.name.get_text() + ',');
    write_list<statement_ptr_t>(interface_declaration.members->statements, [&](const auto& member)
    {
        visit(member);
    });
    write_closing_paren();
}

void AstViewer::visit_interface_field(const InterfaceField& interface_field)
{
    indent_++;
    write_line("InterfaceField(");
    write_line(interface_field.name.get_text() + ',');
    visit(interface_field.type);
    write_line(",");
    write("Const: ");
    write(interface_field.const_keyword.has_value() ? "true" : "false");
    write_closing_paren();
}

void AstViewer::visit_interface_method(const InterfaceMethod& interface_method)
{
    indent_++;
    write_line("InterfaceMethod(");
    write(interface_method.name.get_text());
    write_type_list_clause(interface_method.type_parameters);
    write_line(",");
    write_list<type_ref_ptr_t>(interface_method.parameter_types, [&](const auto& parameter_type)
    {
        visit(parameter_type);
    });
    write_line(",");
    visit(interface_method.return_type);
    write_closing_paren();
}

void AstViewer::visit_enum_declaration(const EnumDeclaration& enum_declaration)
{
    indent_++;
    write_line("EnumDeclaration(");
    write_line(enum_declaration.name.get_text() + ',');
    write_list<statement_ptr_t>(enum_declaration.members->statements, [&](const auto& member)
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
    write_type_list_clause(function_declaration.type_parameters);
    write_line(",");
    write_list<statement_ptr_t>(function_declaration.decorator_list, [&](const auto& decorator)
    {
        visit(decorator);
    });

    if (function_declaration.parameters.has_value())
    {
        write_line(",");
        write_list<statement_ptr_t>(function_declaration.parameters.value()->list, [&](const auto& parameter_type)
        {
            visit(parameter_type);
        });
    }

    if (function_declaration.return_type.has_value())
    {
        write_line(",");
        visit(function_declaration.return_type.value()->type);
    }

    write_line(",");
    if (function_declaration.body->block.has_value())
        visit(*function_declaration.body->block);
    else
        visit(function_declaration.body->expression_body.value()->expression);

    write_line(",");
    write("Async: ");
    write(function_declaration.async_keyword.has_value() ? "true" : "false");
    write_closing_paren();
}

void AstViewer::visit_parameter(const Parameter& parameter)
{
    indent_++;
    write_line("Parameter(");
    write(parameter.name.get_text());
    if (parameter.colon_type.has_value())
    {
        write_line(",");
        visit(parameter.colon_type.value()->type);
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
    write_line(instance_constructor.name.get_text() + ',');
    visit(instance_constructor.colon_type->type);
    if (instance_constructor.clone_target.has_value())
    {
        write_line(",");
        visit(*instance_constructor.clone_target);
    }

    if (instance_constructor.declarators.has_value())
    {
        write_line(",");
        write_list<statement_ptr_t>(instance_constructor.declarators.value()->statements, [&](const auto& property_declarator)
        {
            visit(property_declarator);
        });
    }

    if (instance_constructor.parent.has_value())
    {
        write_line(",");
        visit(*instance_constructor.parent);
    }

    write_closing_paren();
}

void AstViewer::visit_instance_property_declarator(const InstancePropertyDeclarator& instance_property_declarator)
{
    indent_++;
    write_line("InstancePropertyDeclarator(");
    write_line(instance_property_declarator.name.get_text() + ',');
    visit(instance_property_declarator.value);
    write_closing_paren();
}

void AstViewer::visit_instance_name_declarator(const InstanceNameDeclarator& instance_name_declarator)
{
    write("InstanceNameDeclarator(");
    write(instance_name_declarator.name.get_text());
    write(")");
}

void AstViewer::visit_instance_attribute_declarator(const InstanceAttributeDeclarator& instance_attribute_declarator)
{
    indent_++;
    write_line("InstanceAttributeDeclarator(");
    write(instance_attribute_declarator.name.get_text() + ',');
    visit(instance_attribute_declarator.value);
    write_closing_paren();
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
    visit(while_statement.statement);
    write_closing_paren();
}

void AstViewer::visit_repeat(const Repeat& repeat_statement)
{
    indent_++;
    write_line("Repeat(");
    visit(repeat_statement.statement);
    write_line(",");
    visit(repeat_statement.condition);
    write_closing_paren();
}

void AstViewer::visit_for(const For& for_statement)
{
    indent_++;
    write_line("For(");
    write_list<Token>(for_statement.names, [&](const auto& name)
    {
        write(name.get_text());
    });
    write_line(",");
    visit(for_statement.iterable);
    write_line(",");
    visit(for_statement.statement);
    write_closing_paren();
}

void AstViewer::visit_after(const After& after_statement)
{
    indent_++;
    write_line("After(");
    visit(after_statement.time_expression);
    write_line(",");
    visit(after_statement.statement);
    write_closing_paren();
}

void AstViewer::visit_every(const Every& every_statement)
{
    indent_++;
    write_line("Every(");
    visit(every_statement.time_expression);
    write_line(",");
    visit(every_statement.statement);
    write_closing_paren();
}

void AstViewer::visit_match(const Match& match_statement)
{
    indent_++;
    write_line("Match(");
    visit(match_statement.expression);
    write_line(",");
    write_list<statement_ptr_t>(match_statement.cases->statements, [&](const auto& statement)
    {
        visit(statement);
    });
    write_closing_paren();
}

void AstViewer::visit_match_case(const MatchCase& match_case)
{
    indent_++;
    write_line("MatchCase(");
    write_list<expression_ptr_t>(match_case.comparands, [&](const auto& statement)
    {
        visit(statement);
    });
    write_line(",");
    visit(match_case.statement);
    write_closing_paren();
}

void AstViewer::visit_match_else_case(const MatchElseCase& match_else_case)
{
    indent_++;
    write_line("MatchElseCase(");
    if (match_else_case.name.has_value())
        write_line(match_else_case.name->get_text() + ',');

    visit(match_else_case.statement);
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

void AstViewer::visit_decorator(const Decorator& decorator)
{
    indent_++;
    write_line("Decorator(");
    write_line(decorator.name.get_text() + ',');
    write_list<expression_ptr_t>(decorator.arguments, [&](const auto& argument)
    {
        visit(argument);
    });
    write_closing_paren();
}

void AstViewer::visit_primitive_type(const PrimitiveTypeRef& primitive_type)
{
    write("PrimitiveTypeRef(");
    write(primitive_type.keyword.get_text());
    write(")");
}

void AstViewer::visit_literal_type(const LiteralTypeRef& literal_type)
{
    write("LiteralTypeRef(");
    write(literal_type.token.get_text());
    write(")");
}

void AstViewer::visit_type_name(const TypeNameRef& type_name)
{
    indent_++;
    write_line("TypeNameRef(");
    write(type_name.name.get_text());
    write_type_list_clause(type_name.type_arguments);
    write_closing_paren();
}

void AstViewer::visit_nullable_type(const NullableTypeRef& nullable_type)
{
    indent_++;
    write_line("NullableTypeRef(");
    visit(nullable_type.non_nullable_type);
    write_closing_paren();
}

void AstViewer::visit_array_type(const ArrayTypeRef& array_type)
{
    indent_++;
    write_line("ArrayTypeRef(");
    visit(array_type.element_type);
    write_closing_paren();
}

void AstViewer::visit_tuple_type(const TupleTypeRef& tuple_type)
{
    indent_++;
    write_line("TupleTypeRef(");
    write_list<type_ref_ptr_t>(tuple_type.elements, [&](const auto& type_ref)
    {
        visit(type_ref);
    });
    write_closing_paren();
}

void AstViewer::visit_function_type(const FunctionTypeRef& function_type)
{
    indent_++;
    write_line("FunctionTypeRef(");
    if (function_type.type_parameters.has_value())
    {
        write_list<type_ref_ptr_t>(function_type.type_parameters.value()->list, [&](const auto& type_ref)
        {
            visit(type_ref);
        });
        write_line(",");
    }
    write_list<type_ref_ptr_t>(function_type.parameter_types, [&](const auto& type_ref)
    {
        visit(type_ref);
    });
    write_line(",");
    visit(function_type.return_type);
    write_closing_paren();
}

void AstViewer::visit_union_type(const UnionTypeRef& union_type)
{
    indent_++;
    write_line("UnionTypeRef(");
    write_list<type_ref_ptr_t>(union_type.types, [&](const auto& type_ref)
    {
        visit(type_ref);
    });
    write_closing_paren();
}

void AstViewer::visit_intersection_type(const IntersectionTypeRef& intersection_type)
{
    indent_++;
    write_line("IntersectionTypeRef(");
    write_list<type_ref_ptr_t>(intersection_type.types, [&](const auto& type_ref)
    {
        visit(type_ref);
    });
    write_closing_paren();
}


void AstViewer::visit_type_parameter(const TypeParameterRef& type_parameter)
{
    indent_++;
    write_line("TypeParameter(");
    write(type_parameter.name.get_text());

    if (type_parameter.base_type.has_value())
    {
        write_line(",");
        visit(*type_parameter.base_type);
    }
    if (type_parameter.default_type.has_value())
    {
        write_line(",");
        visit(*type_parameter.default_type);
    }

    write_closing_paren();
}