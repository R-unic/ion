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

void Resolver::declare_define(const Token& token)
{
    declare(token);
    define(token);
}

void Resolver::declare_define(const std::string& name, const FileSpan& span)
{
    declare(name, span);
    define(name);
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

void Resolver::visit_block(const Block& block)
{
    push_scope();
    AstVisitor::visit_block(block);
    pop_scope();
}

void Resolver::visit_type_declaration(const TypeDeclaration& type_declaration)
{
    declare(type_declaration.name);
    AstVisitor::visit_type_declaration(type_declaration);
}

void Resolver::visit_variable_declaration(const VariableDeclaration& variable_declaration)
{
    declare(variable_declaration.name);
    AstVisitor::visit_variable_declaration(variable_declaration);
    define(variable_declaration.name);
}

void Resolver::visit_event_declaration(const EventDeclaration& event_declaration)
{
    declare_define(event_declaration.name);
    AstVisitor::visit_event_declaration(event_declaration);
}

void Resolver::visit_interface_declaration(const InterfaceDeclaration& interface_declaration)
{
    declare(interface_declaration.name);
    AstVisitor::visit_interface_declaration(interface_declaration);
}

void Resolver::visit_enum_declaration(const EnumDeclaration& enum_declaration)
{
    declare_define(enum_declaration.name);
    AstVisitor::visit_enum_declaration(enum_declaration);
}

void Resolver::visit_function_declaration(const FunctionDeclaration& function_declaration)
{
    declare_define(function_declaration.name);
    visit_statements(function_declaration.decorator_list);
    visit_type_list_clause(function_declaration.type_parameters);

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
    declare_define(parameter.name);
    if (parameter.colon_type.has_value())
        visit(parameter.colon_type.value()->type);
    if (parameter.equals_value.has_value())
        visit(parameter.equals_value.value()->value);
}

void Resolver::visit_instance_constructor(const InstanceConstructor& instance_constructor)
{
    declare_define(instance_constructor.name);
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
    if (return_statement.expression.has_value())
        visit(*return_statement.expression);
}

void Resolver::visit_while(const While& while_statement)
{
    ContextGuard loop(this, Context::Loop);
    AstVisitor::visit_while(while_statement);
}

void Resolver::visit_repeat(const Repeat& repeat_statement)
{
    ContextGuard loop(this, Context::Loop);
    AstVisitor::visit_repeat(repeat_statement);
}

void Resolver::visit_for(const For& for_statement)
{
    ContextGuard loop(this, Context::Loop);
    AstVisitor::visit_for(for_statement);
}

void Resolver::visit_every(const Every& every_statement)
{
    ContextGuard loop(this, Context::Loop);
    AstVisitor::visit_every(every_statement);
}

void Resolver::visit_import(const Import& import_statement)
{
    // TODO: validate import path
    for (const auto& name : import_statement.names)
        declare_define(name);
}

void Resolver::visit_type_name(const TypeNameRef&)
{
}