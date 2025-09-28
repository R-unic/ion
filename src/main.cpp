#include "ion/parser.h"
#include "ion/source_file.h"
#include "ion/ast/viewer.h"

int main()
{
    const auto file = create_file("test.ion");
    const auto statements = parse(file);
    const auto viewer = new AstViewer;
    logger::info("Running AstViewer on parsed statements...");
    std::cout << '\n';
    viewer->visit(statements);

    return 0;
}