#include "ion/binder.h"

#include "ion/intrinsics.h"
#include "ion/symbols/named_symbol.h"


void Binder::bind_symbol(NamedDeclaration& named_declaration) const
{
    bind_symbol(named_declaration, named_declaration.name);
}

void Binder::bind_symbol(SyntaxNode& node, const Token& token) const
{
    bind_symbol(node, token.get_text());
}

void Binder::bind_symbol(SyntaxNode& node, const std::string& name) const
{
    node.symbol = define_symbol(name);
}

symbol_ptr_t Binder::define_symbol(const std::string& name) const
{
    const auto symbol = std::make_shared<NamedSymbol>(name);
    return define_symbol(symbol);
}

symbol_ptr_t Binder::define_symbol(const symbol_ptr_t& symbol) const
{
    COMPILER_ASSERT(!scopes_.empty(), "Cannot define symbol; scope stack is empty");
    auto scope = scopes_.back();
    scope.insert(symbol);

    return symbol;
}

void Binder::visit_ast(const std::vector<statement_ptr_t>& statements)
{
    ScopedAstVisitor::visit_ast(statements, [&]
    {
        for (const auto& symbol : intrinsic_symbols)
        {
            auto _ = define_symbol(symbol);
            logger::info("Defined intrinsic symbol '" + symbol->to_string() + "' for binder");
        }
    });
}

DEFINE_NAMED_DECLARATION_VISITOR(variable_declaration, VariableDeclaration);
DEFINE_NAMED_DECLARATION_VISITOR(function_declaration, FunctionDeclaration);
DEFINE_NAMED_DECLARATION_VISITOR(event_declaration, EventDeclaration);
DEFINE_NAMED_DECLARATION_VISITOR(instance_constructor, InstanceConstructor);