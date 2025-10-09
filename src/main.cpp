#include "ion/source_file.h"
#include "ion/symbols/named_symbol.h"
#include "ion/parsing/parser.h"
#include "ion/binder.h"
#include "ion/resolver.h"
#include "ion/ast/viewer.h"

int main()
{
    const auto file = create_file("test.ion");
    const auto statements = parse(file);
    const auto viewer = new AstViewer;
    logger::info("Running AstViewer on parsed statements...");
    std::cout << '\n';
    viewer->visit_statements(statements);

    const auto resolver = new Resolver;
    resolver->visit_ast(statements);
    logger::info("Successfully resolved AST");

    const auto binder = new Binder;
    binder->visit_ast(statements);
    logger::info("Successfully bound AST");
    for (const auto& statement : statements)
        if (statement->symbol.has_value())
            std::cout << statement->symbol.value()->to_string() << '\n';
}