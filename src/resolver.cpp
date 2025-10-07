#include "ion/resolver.h"

#define ASSERT_CONTEXT(statement, ctx, report_fn) \
    if (context != ctx) report_fn(statement.get_span());

/** Sets the current context and returns it back to the enclosing context when this struct goes out of scope */
struct ContextGuard
{
    Resolver* resolver;
    ResolverContext enclosing;

    explicit ContextGuard(Resolver* resolver, const ResolverContext context)
        : resolver(resolver)
    {
        enclosing = resolver->context;
        resolver->context = context;
    }

    ~ContextGuard()
    {
        resolver->context = enclosing;
    }
};

bool Resolver::is_declared_in_scope(const std::string& name, const std::unordered_map<std::string, bool>& scope)
{
    return scope.contains(name);
}

void Resolver::define(const Token& identifier)
{
    define(identifier.get_text());
}

void Resolver::define(const std::string& name)
{
    if (scopes_.empty())
        return;

    auto& scope = scopes_.top();
    scope.insert_or_assign(name, true);
}

void Resolver::declare(const Token& identifier)
{
    declare(identifier.get_text(), identifier.span);
}

void Resolver::declare(const std::string& name, const FileSpan& span)
{
    if (scopes_.empty())
        return;

    auto& scope = scopes_.top();
    if (is_declared_in_scope(name, scope))
        report_duplicate_variable(span, name);

    scope.insert_or_assign(name, false);
}

bool Resolver::is_defined(const std::string& name)
{
    auto scopes = scopes_; // intentional copy
    for (auto i = scopes_.size(); i > 0; i--)
    {
        if (const auto& scope = scopes_.top(); is_declared_in_scope(name, scope))
            return scope.at(name);

        scopes_.pop();
    }

    return false;
}

void Resolver::push_scope()
{
    scopes_.emplace();
}

void Resolver::pop_scope()
{
    scopes_.pop();
}

void Resolver::visit_ast(const std::vector<statement_ptr_t>& statements)
{
    push_scope();
    visit_statements(statements);
    pop_scope();
}

void Resolver::visit_type_list(const std::optional<TypeListClause*> type_list)
{
    if (type_list.has_value())
        visit_type_refs(type_list.value()->list);
}

void Resolver::visit_primitive_literal(const PrimitiveLiteral&)
{
}

void Resolver::visit_array_literal(const ArrayLiteral& array_literal)
{
    visit_expressions(array_literal.elements);
}

void Resolver::visit_tuple_literal(const TupleLiteral& tuple_literal)
{
    visit_expressions(tuple_literal.elements);
}

void Resolver::visit_range_literal(const RangeLiteral& range_literal)
{
    visit(range_literal.minimum);
    visit(range_literal.maximum);
}

void Resolver::visit_rgb_literal(const RgbLiteral& rgb_literal)
{
    visit(rgb_literal.r);
    visit(rgb_literal.g);
    visit(rgb_literal.b);
}

void Resolver::visit_hsv_literal(const HsvLiteral& hsv_literal)
{
    visit(hsv_literal.h);
    visit(hsv_literal.s);
    visit(hsv_literal.v);
}

void Resolver::visit_vector_literal(const VectorLiteral& vector_literal)
{
    visit(vector_literal.x);
    visit(vector_literal.y);
    visit(vector_literal.z);
}

void Resolver::visit_interpolated_string(const InterpolatedString& interpolated_string)
{
    visit_expressions(interpolated_string.interpolations);
}

void Resolver::visit_identifier(const Identifier& identifier)
{
    if (scopes_.empty())
        return;

    const auto& scope = scopes_.top();
    const auto name = identifier.get_text();
    if (is_declared_in_scope(name, scope) && scope.at(name) == false)
        report_variable_read_in_own_initializer(identifier.name);
    if (!is_defined(name))
        report_variable_not_found(identifier.name);
}

void Resolver::visit_parenthesized(const Parenthesized& parenthesized)
{
    visit(parenthesized.expression);
}

void Resolver::visit_binary_op(const BinaryOp& binary_op)
{
    visit(binary_op.left);
    visit(binary_op.right);
}

void Resolver::visit_unary_op(const UnaryOp&)
{
}

void Resolver::visit_postfix_unary_op(const PostfixUnaryOp& postfix_unary_op)
{
    visit(postfix_unary_op.operand);
}

void Resolver::visit_assignment_op(const AssignmentOp&)
{
}

void Resolver::visit_ternary_op(const TernaryOp&)
{
}

void Resolver::visit_invocation(const Invocation& invocation)
{
    visit(invocation.callee);
    visit_type_list(invocation.type_arguments);
    visit_expressions(invocation.arguments);
}

void Resolver::visit_type_of(const TypeOf& type_of)
{
    visit(type_of.expression);
}

void Resolver::visit_name_of(const NameOf&)
{
}

void Resolver::visit_await(const Await& await)
{
    visit(await.expression);
}

void Resolver::visit_member_access(const MemberAccess& member_access)
{
    visit(member_access.expression);
}

void Resolver::visit_optional_member_access(const OptionalMemberAccess& optional_member_access)
{
    visit(optional_member_access.expression);
}

void Resolver::visit_element_access(const ElementAccess& element_access)
{
    visit(element_access.expression);
    visit(element_access.index_expression);
}

void Resolver::visit_expression_statement(const ExpressionStatement& expression_statement)
{
    visit(expression_statement.expression);
}

void Resolver::visit_block(const Block& block)
{
    visit_statements(block.braced_statement_list->statements);
}

void Resolver::visit_type_declaration(const TypeDeclaration& type_declaration)
{
    declare(type_declaration.name);
    visit_type_list(type_declaration.type_parameters);
    visit(type_declaration.type);
}

void Resolver::visit_variable_declaration(const VariableDeclaration& variable_declaration)
{
    declare(variable_declaration.name);
    if (variable_declaration.colon_type.has_value())
        visit(variable_declaration.colon_type.value()->type);
    if (variable_declaration.equals_value.has_value())
        visit(variable_declaration.equals_value.value()->value);

    define(variable_declaration.name);
}

void Resolver::visit_event_declaration(const EventDeclaration& event_declaration)
{
    declare(event_declaration.name);
    visit_type_list(event_declaration.type_parameters);
    define(event_declaration.name);
}

void Resolver::visit_interface_declaration(const InterfaceDeclaration& interface_declaration)
{
    declare(interface_declaration.name);
}

void Resolver::visit_interface_field(const InterfaceField&)
{
}

void Resolver::visit_interface_method(const InterfaceMethod&)
{
}

void Resolver::visit_enum_declaration(const EnumDeclaration& enum_declaration)
{
    declare(enum_declaration.name);
    define(enum_declaration.name);
    visit_statements(enum_declaration.members->statements);
}

void Resolver::visit_enum_member(const EnumMember&)
{
}

void Resolver::visit_function_declaration(const FunctionDeclaration& function_declaration)
{
    declare(function_declaration.name);
    define(function_declaration.name);
    visit_statements(function_declaration.decorator_list);
    visit_type_list(function_declaration.type_parameters);

    push_scope();
    visit_statements(function_declaration.parameters.value()->list);
    if (function_declaration.return_type.has_value())
        visit(function_declaration.return_type.value()->type);

    ContextGuard fn(this, ResolverContext::Function);
    if (function_declaration.body->block.has_value())
        visit(*function_declaration.body->block);
    else
        visit(function_declaration.body->expression_body.value()->expression);

    pop_scope();
}

void Resolver::visit_parameter(const Parameter& parameter)
{
    declare(parameter.name);
    define(parameter.name);
    if (parameter.colon_type.has_value())
        visit(parameter.colon_type.value()->type);
    if (parameter.equals_value.has_value())
        visit(parameter.equals_value.value()->value);
}

void Resolver::visit_instance_constructor(const InstanceConstructor& instance_constructor)
{
    declare(instance_constructor.name);
    define(instance_constructor.name);
}

void Resolver::visit_instance_property_declarator(const InstancePropertyDeclarator&)
{
}

void Resolver::visit_instance_name_declarator(const InstanceNameDeclarator&)
{
}

void Resolver::visit_instance_attribute_declarator(const InstanceAttributeDeclarator&)
{
}

void Resolver::visit_instance_tag_declarator(const InstanceTagDeclarator&)
{
}

void Resolver::visit_break(const Break& break_statement)
{
    ASSERT_CONTEXT(break_statement, Context::Loop, report_invalid_break);
}

void Resolver::visit_continue(const Continue& continue_statement)
{
    ASSERT_CONTEXT(continue_statement, Context::Loop, report_invalid_continue);
}

void Resolver::visit_return(const Return& return_statement)
{
    ASSERT_CONTEXT(return_statement, Context::Function, report_invalid_return);
}

void Resolver::visit_if(const If& if_statement)
{
    visit(if_statement.condition);
    visit(if_statement.then_branch);
    if (if_statement.else_branch.has_value())
        visit(*if_statement.else_branch);
}

void Resolver::visit_while(const While& while_statement)
{
    ContextGuard loop(this, Context::Loop);
    visit(while_statement.condition);
    visit(while_statement.statement);
}

void Resolver::visit_repeat(const Repeat& repeat_statement)
{
    ContextGuard loop(this, Context::Loop);
    visit(repeat_statement.statement);
    visit(repeat_statement.condition);
}

void Resolver::visit_for(const For& for_statement)
{
    ContextGuard loop(this, Context::Loop);
    visit(for_statement.iterable);
    visit(for_statement.statement);
}

void Resolver::visit_after(const After& after_statement)
{
    visit(after_statement.time_expression);
    visit(after_statement.statement);
}

void Resolver::visit_every(const Every& every_statement)
{
    ContextGuard loop(this, Context::Loop);
    visit(every_statement.time_expression);
    visit(every_statement.statement);
}

void Resolver::visit_match(const Match& match_statement)
{
    visit(match_statement.expression);
    visit_statements(match_statement.cases->statements);
}

void Resolver::visit_match_case(const MatchCase& match_case)
{
    visit_expressions(match_case.comparands);
    visit(match_case.statement);
}

void Resolver::visit_match_else_case(const MatchElseCase& match_else_case)
{
    visit(match_else_case.statement);
}

void Resolver::visit_import(const Import&)
{
}

void Resolver::visit_export(const Export& export_statement)
{
    visit(export_statement.statement);
}

void Resolver::visit_decorator(const Decorator& decorator)
{
    visit_expressions(decorator.arguments);
}

void Resolver::visit_primitive_type(const PrimitiveTypeRef&)
{
}

void Resolver::visit_literal_type(const LiteralTypeRef&)
{
}

void Resolver::visit_type_name(const TypeNameRef&)
{
}

void Resolver::visit_nullable_type(const NullableTypeRef&)
{
}

void Resolver::visit_array_type(const ArrayTypeRef&)
{
}

void Resolver::visit_tuple_type(const TupleTypeRef&)
{
}

void Resolver::visit_function_type(const FunctionTypeRef&)
{
}

void Resolver::visit_union_type(const UnionTypeRef&)
{
}

void Resolver::visit_intersection_type(const IntersectionTypeRef&)
{
}

void Resolver::visit_type_parameter(const TypeParameterRef&)
{
}