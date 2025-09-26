#include "ion/parser.h"
#include "ion/source_file.h"
#include "ion/ast/viewer.h"

int main(int argc, char* argv[])
{
    const auto file = create_file("test.ion");
    const auto statements = parse(file);
    const auto viewer = new AstViewer;
    viewer->visit(statements);
    
    return 0;
}