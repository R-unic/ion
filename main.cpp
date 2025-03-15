#include <functional>
#include <iostream>
#include <ostream>

#include "source_file.h"

int main(int argc, char* argv[])
{
    const auto file = source_file("test.ion");
    const auto ast = file.parse();
    
    std::cout << &ast << '\n';
    
    return 0;
}
