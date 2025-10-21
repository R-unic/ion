#include "ion/binder.h"

#include "ion/intrinsics.h"
#include "ion/symbols/named_symbol.h"
#include "ion/symbols/type_declaration_symbol.h"
#include "ion/types/type_name.h"

void Binder::bind_declaration_symbol(NamedDeclaration* named_declaration)
{
    named_declaration->symbol = define_declaration_symbol(named_declaration);
}

void Binder::bind_type_declaration_symbol(NamedDeclaration* named_declaration, type_ptr_t& type)
{
    named_declaration->symbol = define_type_declaration_symbol(named_declaration, type);
}

void Binder::bind_named_symbol(SyntaxNode& node, const Token& token)
{
    bind_named_symbol(node, token.get_text());
}

void Binder::bind_named_symbol(SyntaxNode& node, const std::string& name)
{
    node.symbol = define_named_symbol(name);
}

void Binder::bind_empty_symbol(SyntaxNode& node)
{
    const auto symbol_ptr = std::make_shared<Symbol>();
    node.symbol = define_symbol(symbol_ptr);
}

declaration_symbol_ptr_t Binder::define_declaration_symbol(const NamedDeclaration* named_declaration)
{
    const auto symbol = std::make_shared<DeclarationSymbol>(named_declaration->name.get_text(), named_declaration);
    return define_symbol(symbol);
}

declaration_symbol_ptr_t Binder::define_type_declaration_symbol(const NamedDeclaration* named_declaration, type_ptr_t& type)
{
    const auto symbol = std::make_shared<TypeDeclarationSymbol>(named_declaration->name.get_text(), std::move(type), named_declaration);
    return define_symbol(symbol);
}

named_symbol_ptr_t Binder::define_type_symbol(const std::string& name, type_ptr_t& type)
{
    const auto symbol = std::make_shared<TypeSymbol>(name, std::move(type));
    return define_symbol(symbol);
}

named_symbol_ptr_t Binder::define_named_symbol(const std::string& name)
{
    const auto symbol = std::make_shared<NamedSymbol>(name);
    return define_symbol(symbol);
}

template <typename SymbolTy>
SymbolTy Binder::define_symbol(const SymbolTy& symbol)
{
    COMPILER_ASSERT(!scopes_.empty(), "Cannot define symbol; scope stack is empty");
    auto& scope = scopes_.back();
    scope.insert(static_cast<symbol_ptr_t>(symbol));

    return symbol;
}

std::optional<named_symbol_ptr_t> Binder::find_named_symbol(const std::string& name) const
{
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it)
        for (const auto& symbol_ptr : *it)
            if (auto named_symbol = std::dynamic_pointer_cast<NamedSymbol>(symbol_ptr))
                if (!named_symbol->is_type_symbol() && !named_symbol->is_type_declaration_symbol() && named_symbol->name == name)
                    return named_symbol;

    return std::nullopt;
}

std::optional<named_symbol_ptr_t> Binder::find_type_symbol(const std::string& name) const
{
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it)
        for (const auto& symbol_ptr : *it)
            if (auto type_symbol = std::dynamic_pointer_cast<TypeSymbol>(symbol_ptr))
                if (type_symbol->name == name)
                    return type_symbol;

    return std::nullopt;
}

void Binder::visit_ast(const std::vector<statement_ptr_t>& statements)
{
    ScopedAstVisitor::visit_ast(statements, [&]
    {
        for (const auto& symbol : intrinsic_symbols)
            logger::info("Defined intrinsic symbol '" + define_symbol(symbol)->to_string() + "' for binder");
    });
}


void Binder::visit_identifier(Identifier& identifier)
{
    if (const auto symbol_opt = find_named_symbol(identifier.get_text()); symbol_opt.has_value())
    {
        if (const auto& symbol = *symbol_opt; !symbol->is_declaration_symbol())
            identifier.symbol = symbol;
        else if (!symbol->is_type_declaration_symbol())
        {
            const auto named_symbol = define_named_symbol(symbol->name);
            named_symbol->declaring_symbol = symbol;
            identifier.symbol = named_symbol;
        }
    }

    ScopedAstVisitor::visit_identifier(identifier);
}

void Binder::visit_expression_statement(ExpressionStatement& expression_statement)
{
    ScopedAstVisitor::visit_expression_statement(expression_statement);
    expression_statement.symbol = expression_statement.expression->symbol;
}

void Binder::visit_variable_declaration(VariableDeclaration& variable_declaration)
{
    AstVisitor::visit_variable_declaration(variable_declaration);
    const auto type = variable_declaration.colon_type.has_value()
                          ? Type::from(variable_declaration.colon_type.value()->type)
                          : std::optional<type_ptr_t>(std::nullopt);

    const auto symbol = std::make_shared<DeclarationSymbol>(variable_declaration.name.get_text(), &variable_declaration, type);
    variable_declaration.symbol = define_symbol(symbol);
}

void Binder::visit_enum_declaration(EnumDeclaration& enum_declaration)
{
    AstVisitor::visit_enum_declaration(enum_declaration);
    bind_declaration_symbol(&enum_declaration);

    auto enum_type = number_type.as_shared(); // TODO: union of literal types
    const auto _ = define_type_declaration_symbol(&enum_declaration, enum_type);
}

void Binder::visit_type_name(TypeNameRef& type_name_ref)
{
    if (const auto symbol_opt = find_type_symbol(type_name_ref.get_text()); symbol_opt.has_value())
    {
        if (const auto& symbol = *symbol_opt; !symbol->is_type_declaration_symbol())
            type_name_ref.symbol = symbol;
        else
        {
            type_ref_ptr_t type_ref_ptr = std::make_unique<TypeNameRef>(type_name_ref);
            auto type = Type::from(type_ref_ptr);
            const auto named_symbol = define_type_symbol(symbol->name, type);
            named_symbol->declaring_symbol = symbol;
            type_name_ref.symbol = named_symbol;
        }
    }

    ScopedAstVisitor::visit_type_name(type_name_ref);
}

DEFINE_EMPTY_SYMBOL_VISITOR(primitive_literal, PrimitiveLiteral)
DEFINE_EMPTY_SYMBOL_VISITOR(array_literal, ArrayLiteral);
DEFINE_EMPTY_SYMBOL_VISITOR(tuple_literal, TupleLiteral);
DEFINE_EMPTY_SYMBOL_VISITOR(range_literal, RangeLiteral);
DEFINE_EMPTY_SYMBOL_VISITOR(rgb_literal, RgbLiteral);
DEFINE_EMPTY_SYMBOL_VISITOR(hsv_literal, HsvLiteral);
DEFINE_EMPTY_SYMBOL_VISITOR(vector_literal, VectorLiteral);
DEFINE_EMPTY_SYMBOL_VISITOR(interpolated_string, InterpolatedString);
DEFINE_EMPTY_SYMBOL_VISITOR(parenthesized, Parenthesized);
DEFINE_EMPTY_SYMBOL_VISITOR(binary_op, BinaryOp);
DEFINE_EMPTY_SYMBOL_VISITOR(assignment_op, AssignmentOp);
DEFINE_EMPTY_SYMBOL_VISITOR(unary_op, UnaryOp);
DEFINE_EMPTY_SYMBOL_VISITOR(postfix_unary_op, PostfixUnaryOp);
DEFINE_EMPTY_SYMBOL_VISITOR(invocation, Invocation);
DEFINE_EMPTY_SYMBOL_VISITOR(member_access, MemberAccess);
DEFINE_EMPTY_SYMBOL_VISITOR(optional_member_access, OptionalMemberAccess);
DEFINE_EMPTY_SYMBOL_VISITOR(element_access, ElementAccess);
DEFINE_EMPTY_SYMBOL_VISITOR(await, Await);
DEFINE_EMPTY_SYMBOL_VISITOR(name_of, NameOf);
DEFINE_EMPTY_SYMBOL_VISITOR(type_of, TypeOf);

DEFINE_SCOPED_DECLARATION_VISITOR(function_declaration, FunctionDeclaration);
DEFINE_SCOPED_DECLARATION_VISITOR(event_declaration, EventDeclaration);
DEFINE_SCOPED_DECLARATION_VISITOR(instance_constructor, InstanceConstructor);
DEFINE_TYPE_DECLARATION_VISITOR(type_declaration, TypeDeclaration, Type::from(type_declaration.type));
DEFINE_TYPE_DECLARATION_VISITOR(interface_declaration, InterfaceDeclaration, Type::from_interface(interface_declaration))