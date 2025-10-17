#include <iostream>

#include "ion/compiler.h"
#include "ion/source_file.h"

int main()
{
    auto compiler = Compiler(create_file("test.ion"));
    compiler.emit();

    for (const auto& file : compiler.files)
        for (const auto& statement : file.statements)
            if (statement->symbol.has_value())
                std::cout << typeid(*statement).name() << ": " << statement->symbol.value()->to_string() << '\n';
}