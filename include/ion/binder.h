#pragma once
#include <set>

#include "ion/ast/visitor.h"
#include "ion/logger.h"
#include "symbols/named_symbol.h"
#include "symbols/declaration_symbol.h"

#define DEFINE_TYPE_DECLARATION_VISITOR(name, name_capitalized, type_value) \
    void Binder::visit_##name(name_capitalized& name) \
    { \
        AstVisitor::visit_##name(name); \
        auto type = type_value; \
        bind_type_declaration_symbol(&name, type); \
    }

#define DEFINE_DECLARATION_VISITOR(name, name_capitalized) \
    void Binder::visit_##name(name_capitalized& name) \
    { \
        AstVisitor::visit_##name(name); \
        bind_declaration_symbol(&name); \
    }

#define DEFINE_SCOPED_DECLARATION_VISITOR(name, name_capitalized) \
    void Binder::visit_##name(name_capitalized& name) \
    { \
        bind_declaration_symbol(&name); \
        push_scope(); \
        AstVisitor::visit_##name(name); \
        pop_scope(); \
    }

#define DEFINE_EMPTY_SYMBOL_VISITOR(name, name_capitalized) \
    void Binder::visit_##name(name_capitalized& name) \
    { \
        bind_empty_symbol(name); \
        AstVisitor::visit_##name(name); \
    }

class Binder final : public ScopedAstVisitor<void, std::set<symbol_ptr_t>>
{
public:
    Binder()
    {
        logger::info("Created binder");
    }

    void bind_declaration_symbol(NamedDeclaration*);
    void bind_type_declaration_symbol(NamedDeclaration*, type_ptr_t&);
    void bind_named_symbol(SyntaxNode&, const Token&);
    void bind_named_symbol(SyntaxNode&, const std::string&);
    void bind_empty_symbol(SyntaxNode&);
    [[nodiscard]] declaration_symbol_ptr_t define_declaration_symbol(const NamedDeclaration*);
    [[nodiscard]] declaration_symbol_ptr_t define_type_declaration_symbol(const NamedDeclaration*, type_ptr_t&);
    named_symbol_ptr_t define_type_symbol(const std::string& name, type_ptr_t& type);
    [[nodiscard]] named_symbol_ptr_t define_named_symbol(const std::string&);
    template <typename SymbolTy = symbol_ptr_t>
    [[nodiscard]] SymbolTy define_symbol(const SymbolTy&);
    [[nodiscard]] std::optional<named_symbol_ptr_t> find_named_symbol(const std::string&) const;
    [[nodiscard]] std::optional<named_symbol_ptr_t> find_type_symbol(const std::string& name) const;

    void visit_ast(const std::vector<statement_ptr_t>&) override;

    void visit_identifier(Identifier&) override;
    void visit_primitive_literal(PrimitiveLiteral&) override;
    void visit_array_literal(ArrayLiteral&) override;
    void visit_tuple_literal(TupleLiteral&) override;
    void visit_range_literal(RangeLiteral&) override;
    void visit_rgb_literal(RgbLiteral&) override;
    void visit_hsv_literal(HsvLiteral&) override;
    void visit_vector_literal(VectorLiteral&) override;
    void visit_interpolated_string(InterpolatedString&) override;
    void visit_parenthesized(Parenthesized&) override;
    void visit_binary_op(BinaryOp&) override;
    void visit_assignment_op(AssignmentOp&) override;
    void visit_unary_op(UnaryOp&) override;
    void visit_postfix_unary_op(PostfixUnaryOp&) override;
    void visit_invocation(Invocation&) override;
    void visit_member_access(MemberAccess&) override;
    void visit_optional_member_access(OptionalMemberAccess&) override;
    void visit_element_access(ElementAccess&) override;
    void visit_await(Await&) override;
    void visit_name_of(NameOf&) override;
    void visit_type_of(TypeOf&) override;

    void visit_expression_statement(ExpressionStatement&) override;
    void visit_variable_declaration(VariableDeclaration&) override;
    void visit_function_declaration(FunctionDeclaration&) override;
    void visit_event_declaration(EventDeclaration&) override;
    void visit_enum_declaration(EnumDeclaration& enum_declaration) override;
    void visit_instance_constructor(InstanceConstructor&) override;
    void visit_type_declaration(TypeDeclaration&) override;
    void visit_interface_declaration(InterfaceDeclaration&) override;

    void visit_type_name(TypeNameRef&) override;
};