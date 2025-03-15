#include <functional>
#include <iostream>
#include <ostream>

#include "ast_viewer.h"
#include "source_file.h"

int main(int argc, char* argv[])
{
    const auto file = source_file("test.ion");
    auto ast = file.parse();
    const auto ast_viewer = ::ast_viewer();
    
    std::cout << ast_viewer.visit(ast) << '\n';
    
    return 0;
}
