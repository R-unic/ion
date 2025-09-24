#include <iostream>
#include <ostream>
#include <vector>

#include "diagnostics.h"
#include "lexer.h"
#include "token.h"

int main(int argc, char* argv[])
{
    const auto file = create_file("test.ion");
    const auto tokens = tokenize(file);
    for (const auto& token : tokens)
    {
        std::cout << std::to_string(static_cast<int>(token.kind)) << '\n';
    }
    
    return 0;
}
