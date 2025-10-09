#include "ion/binder.h"

void Binder::bind_symbol(NamedDeclaration& named_declaration)
{
    bind_symbol(named_declaration, named_declaration.name);
}

void Binder::bind_symbol(SyntaxNode& node, const Token& token)
{
    bind_symbol(node, token.get_text());
}

void Binder::bind_symbol(SyntaxNode& node, const std::string& name)
{
    node.symbol = std::make_shared<NamedSymbol>(name);
}

void Binder::visit_variable_declaration(VariableDeclaration& variable_declaration)
{
    bind_symbol(variable_declaration);
    AstVisitor::visit_variable_declaration(variable_declaration);
}