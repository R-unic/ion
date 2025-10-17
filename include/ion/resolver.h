#pragma once
#include <set>

#include "ion/ast/visitor.h"
#include "ion/logger.h"

#define ASSERT_CONTEXT(node, report_fn, ctx) \
    if (context != ctx) report_fn(node.get_span());

#define ASSERT_ANY_CONTEXT(node, report_fn, contexts) \
    if (!contexts.contains(context)) report_fn(node.get_span());

#define DUPLICATE_MEMBER_CHECK_CUSTOM_NAME(node, set, field_type, name) \
    if (set.contains(name)) \
        report_duplicate_member(node.get_span(), field_type); \
    set.insert(name)

#define DUPLICATE_MEMBER_CHECK(node, set, field_type) \
    const auto name = node.name.get_text(); \
    DUPLICATE_MEMBER_CHECK_CUSTOM_NAME(node, set, field_type, name)

#define DEFINE_LOOP_VISITOR(name, name_capitalized) \
    void Resolver::visit_##name(name_capitalized& name##_statement) \
    { \
        ContextGuard loop(this, Context::Loop); \
        AstVisitor::visit_##name(name##_statement); \
    }

enum class ResolverContext
{
    Global,
    Block,
    Loop,
    Function,
    AsyncFunction
};

const std::set function_contexts = { ResolverContext::Function, ResolverContext::AsyncFunction };

class Resolver final : public ScopedAstVisitor<void, std::unordered_map<std::string, bool>>
{
    using Context = ResolverContext;
    std::set<std::string> used_interface_members = {};
    std::set<std::string> used_instance_properties = {};
    std::set<std::string> used_instance_attributes = {};
    std::set<std::string> used_instance_tags = {};

public:
    Context context = Context::Global;

    Resolver()
    {
        logger::info("Created resolver");
    }

    static bool is_declared_in_scope(const std::string&, const std::unordered_map<std::string, bool>&);

    void define(const Token&);
    void define(const std::string&);
    void declare(const Token&);
    void declare(const std::string&, const FileSpan&);
    void declare_define(const Token&);
    void declare_define(const std::string&, const FileSpan&);
    [[nodiscard]] bool is_defined(const std::string&) const;
    void resolve_name(const Token&) const;
    void define_intrinsic_name(const std::string&);
    void visit_ast(const std::vector<statement_ptr_t>& statements) override;

    void visit_identifier(Identifier&) override;
    void visit_await(Await&) override;

    void visit_block(Block&) override;
    void visit_type_declaration(TypeDeclaration&) override;
    void visit_variable_declaration(VariableDeclaration&) override;
    void visit_event_declaration(EventDeclaration&) override;
    void visit_enum_declaration(EnumDeclaration&) override;
    void visit_interface_declaration(InterfaceDeclaration&) override;
    void visit_interface_field(InterfaceField&) override;
    void visit_interface_method(InterfaceMethod&) override;
    void visit_function_declaration(FunctionDeclaration&) override;
    void visit_parameter(Parameter&) override;
    void visit_instance_constructor(InstanceConstructor&) override;
    void visit_instance_attribute_declarator(InstanceAttributeDeclarator&) override;
    void visit_instance_name_declarator(InstanceNameDeclarator&) override;
    void visit_instance_property_declarator(InstancePropertyDeclarator&) override;
    void visit_instance_tag_declarator(InstanceTagDeclarator&) override;
    void visit_break(Break&) override;
    void visit_continue(Continue&) override;
    void visit_return(Return&) override;
    void visit_while(While&) override;
    void visit_repeat(Repeat&) override;
    void visit_for(For&) override;
    void visit_every(Every&) override;
    void visit_import(Import&) override;

    void visit_type_name(TypeNameRef&) override;
    void visit_type_parameter(TypeParameterRef&) override;
};