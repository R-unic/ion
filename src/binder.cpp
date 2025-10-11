#include "ion/binder.h"

#include "ion/intrinsics.h"
#include "ion/symbols/named_symbol.h"

void Binder::bind_declaration_symbol(NamedDeclaration* named_declaration)
{
    named_declaration->symbol = define_declaration_symbol(named_declaration);
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
    for (auto i = scopes_.size(); i > 0; i--)
    {
        logger::info("Symbols in scope: " + std::to_string(scopes_.at(i - 1).size()));

        for (const auto& symbol_ptr : scopes_.at(i - 1))
        {
            logger::info("Found symbol: " + symbol_ptr->to_string());
            if (!symbol_ptr->is_named_symbol())
                continue;

            if (const auto symbol = dynamic_cast<NamedSymbol*>(symbol_ptr.get()); symbol->name == name)
                return std::static_pointer_cast<NamedSymbol>(symbol_ptr);
        }
    }

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
        else
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

DEFINE_EMPTY_SYMBOL_VISITOR(primitive_literal, PrimitiveLiteral);
DEFINE_EMPTY_SYMBOL_VISITOR(array_literal, ArrayLiteral);
DEFINE_EMPTY_SYMBOL_VISITOR(tuple_literal, TupleLiteral);
DEFINE_EMPTY_SYMBOL_VISITOR(range_literal, RangeLiteral);
DEFINE_EMPTY_SYMBOL_VISITOR(rgb_literal, RgbLiteral);
DEFINE_EMPTY_SYMBOL_VISITOR(hsv_literal, HsvLiteral);
DEFINE_EMPTY_SYMBOL_VISITOR(vector_literal, VectorLiteral)
DEFINE_EMPTY_SYMBOL_VISITOR(interpolated_string, InterpolatedString)
DEFINE_EMPTY_SYMBOL_VISITOR(parenthesized, Parenthesized)
DEFINE_EMPTY_SYMBOL_VISITOR(binary_op, BinaryOp)
DEFINE_EMPTY_SYMBOL_VISITOR(assignment_op, AssignmentOp)
DEFINE_EMPTY_SYMBOL_VISITOR(unary_op, UnaryOp)
DEFINE_EMPTY_SYMBOL_VISITOR(postfix_unary_op, PostfixUnaryOp)
DEFINE_EMPTY_SYMBOL_VISITOR(invocation, Invocation)
DEFINE_EMPTY_SYMBOL_VISITOR(member_access, MemberAccess)
DEFINE_EMPTY_SYMBOL_VISITOR(optional_member_access, OptionalMemberAccess)
DEFINE_EMPTY_SYMBOL_VISITOR(element_access, ElementAccess)
DEFINE_EMPTY_SYMBOL_VISITOR(await, Await)
DEFINE_EMPTY_SYMBOL_VISITOR(name_of, NameOf)
DEFINE_EMPTY_SYMBOL_VISITOR(type_of, TypeOf)

DEFINE_DECLARATION_VISITOR(variable_declaration, VariableDeclaration);
DEFINE_DECLARATION_VISITOR(function_declaration, FunctionDeclaration);
DEFINE_DECLARATION_VISITOR(event_declaration, EventDeclaration);
DEFINE_DECLARATION_VISITOR(instance_constructor, InstanceConstructor);