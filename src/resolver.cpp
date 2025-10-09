#include "ion/resolver.h"

#include "ion/intrinsics.h"

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
    COMPILER_ASSERT(identifier.is_kind(SyntaxKind::Identifier), "Expected identifier token");
    define(identifier.get_text());
}

void Resolver::define(const std::string& name)
{
    if (scopes_.empty())
        return;

    auto& scope = scopes_.back();
    scope.insert_or_assign(name, true);
}

void Resolver::declare(const Token& identifier)
{
    COMPILER_ASSERT(identifier.is_kind(SyntaxKind::Identifier), "Expected identifier token");
    declare(identifier.get_text(), identifier.span);
}

void Resolver::declare(const std::string& name, const FileSpan& span)
{
    if (scopes_.empty())
        return;

    auto& scope = scopes_.back();
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

bool Resolver::is_defined(const std::string& name) const
{
    auto scopes = scopes_; // intentional copy
    for (auto i = scopes_.size(); i > 0; i--)
        if (const auto& scope = scopes_.at(i - 1); is_declared_in_scope(name, scope))
            return scope.at(name);

    return false;
}

void Resolver::resolve_name(const Token& name) const
{
    if (scopes_.empty())
        return;

    const auto& scope = scopes_.back();
    const auto text = name.get_text();
    if (is_declared_in_scope(text, scope) && scope.at(text) == false)
        report_variable_read_in_own_initializer(name);
    if (!is_defined(text))
        report_variable_not_found(name);
}

void Resolver::define_intrinsic_name(const std::string& name)
{
    declare_define(Token { .kind = SyntaxKind::Identifier, .text = name });
    logger::info("Defined intrinsic name '" + name + "' for resolver");
}

void Resolver::visit_ast(const std::vector<statement_ptr_t>& statements)
{
    ScopedAstVisitor::visit_ast(statements, [&]
    {
        for (const auto& symbol : intrinsic_symbols)
            define_intrinsic_name(symbol->name);
    });
}

void Resolver::visit_identifier(Identifier& identifier)
{
    resolve_name(identifier.name);
}

void Resolver::visit_await(Await& await)
{
    ASSERT_CONTEXT(await, report_invalid_await, Context::AsyncFunction);
    AstVisitor::visit_await(await);
}

void Resolver::visit_block(Block& block)
{
    push_scope();
    AstVisitor::visit_block(block);
    pop_scope();
}

void Resolver::visit_type_declaration(TypeDeclaration& type_declaration)
{
    declare(type_declaration.name);
    AstVisitor::visit_type_declaration(type_declaration);
    define(type_declaration.name);
}

void Resolver::visit_variable_declaration(VariableDeclaration& variable_declaration)
{
    declare(variable_declaration.name);
    AstVisitor::visit_variable_declaration(variable_declaration);
    define(variable_declaration.name);
}

void Resolver::visit_event_declaration(EventDeclaration& event_declaration)
{
    declare_define(event_declaration.name);
    push_scope();
    AstVisitor::visit_event_declaration(event_declaration);
    pop_scope();
}

void Resolver::visit_enum_declaration(EnumDeclaration& enum_declaration)
{
    declare_define(enum_declaration.name);
    AstVisitor::visit_enum_declaration(enum_declaration);
}

void Resolver::visit_interface_declaration(InterfaceDeclaration& interface_declaration)
{
    declare_define(interface_declaration.name);
    AstVisitor::visit_interface_declaration(interface_declaration);
}

void Resolver::visit_interface_field(InterfaceField& interface_field)
{
    DUPLICATE_MEMBER_CHECK(interface_field, used_interface_members, "interface member");
    AstVisitor::visit_interface_field(interface_field);
}

void Resolver::visit_interface_method(InterfaceMethod& interface_method)
{
    DUPLICATE_MEMBER_CHECK(interface_method, used_interface_members, "interface member");
    AstVisitor::visit_interface_method(interface_method);
}

void Resolver::visit_function_declaration(FunctionDeclaration& function_declaration)
{
    declare_define(function_declaration.name);
    push_scope();
    AstVisitor::visit_function_declaration(function_declaration);
    pop_scope();
}

void Resolver::visit_parameter(Parameter& parameter)
{
    declare_define(parameter.name);
    AstVisitor::visit_parameter(parameter);
}

void Resolver::visit_instance_constructor(InstanceConstructor& instance_constructor)
{
    declare_define(instance_constructor.name);
    AstVisitor::visit_instance_constructor(instance_constructor);
}

void Resolver::visit_instance_attribute_declarator(InstanceAttributeDeclarator& instance_attribute_declarator)
{
    DUPLICATE_MEMBER_CHECK(instance_attribute_declarator, used_instance_attributes, "instance attribute");
    AstVisitor::visit_instance_attribute_declarator(instance_attribute_declarator);
}

void Resolver::visit_instance_name_declarator(InstanceNameDeclarator& instance_name_declarator)
{
    DUPLICATE_MEMBER_CHECK_CUSTOM_NAME(instance_name_declarator, used_instance_properties, "instance name property", "Name");
    AstVisitor::visit_instance_name_declarator(instance_name_declarator);
}

void Resolver::visit_instance_property_declarator(InstancePropertyDeclarator& instance_property_declarator)
{
    DUPLICATE_MEMBER_CHECK(instance_property_declarator, used_instance_properties, "instance property");
    AstVisitor::visit_instance_property_declarator(instance_property_declarator);
}

void Resolver::visit_instance_tag_declarator(InstanceTagDeclarator& instance_tag_declarator)
{
    DUPLICATE_MEMBER_CHECK(instance_tag_declarator, used_instance_tags, "instance tag");
    AstVisitor::visit_instance_tag_declarator(instance_tag_declarator);
}

void Resolver::visit_break(Break& break_statement)
{
    ASSERT_CONTEXT(break_statement, report_invalid_break, Context::Loop);
}

void Resolver::visit_continue(Continue& continue_statement)
{
    ASSERT_CONTEXT(continue_statement, report_invalid_continue, Context::Loop);
}

void Resolver::visit_return(Return& return_statement)
{
    ASSERT_ANY_CONTEXT(return_statement, report_invalid_return, function_contexts);
    AstVisitor::visit_return(return_statement);
}

// @formatter:off
DEFINE_LOOP_VISITOR(while, While);
DEFINE_LOOP_VISITOR(repeat, Repeat);
DEFINE_LOOP_VISITOR(for, For);
DEFINE_LOOP_VISITOR(every, Every);
// @formatter:on

void Resolver::visit_import(Import& import_statement)
{
    // TODO: validate import path
    for (const auto& name : import_statement.names)
        declare_define(name);
}

void Resolver::visit_type_name(TypeNameRef& type_name)
{
    resolve_name(type_name.name);
}

void Resolver::visit_type_parameter(TypeParameterRef& type_parameter)
{
    declare(type_parameter.name);
    AstVisitor::visit_type_parameter(type_parameter);
    define(type_parameter.name);
}