#include "ion/source_file.h"
#include "ion/ast/viewer.h"
#include "../include/ion/parsing/parser.h"
#include "ion/resolver.h"

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

    return 0;
}